/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <spine/include/provider-backbone-port.h>
#include <e3net/include/e3iface-inventory.h>
#include <e3infra/include/node.h>
#include <e3infra/include/mbuf-delivery.h>
#include <e3infra/include/e3-init.h>
#include <e3infra/include/e3-log.h>
#include <rte_ether.h>
#include <spine/include/spine-label-fib.h>
#include <spine/include/spine-label-mnexthop.h>
#include <e3infra/include/lcore-extension.h>
#include <e3infra/include/util.h>
#include <rte_malloc.h>
#include <e3net/include/mpls-util.h>
#include <e3net/include/common-nhlfe.h>
#include <e3infra/include/malloc-wrapper.h>

#define PBP_NODE_BURST_SIZE 48

extern struct e3iface_role_def  role_defs[E3IFACE_ROLE_MAX_ROLES];

static int null_capability_check(int port_id)
{
	return 0;
}




inline uint64_t _process_pbp_input_packet(struct rte_mbuf * mbuf,
											struct pbp_cache_entry * pbp_cache,
											struct pbp_private     * priv)
{
	uint64_t fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_DROP,0);
	uint32_t label;
	uint32_t cache_index;
	struct E3Interface * psrc_iface;
	struct ether_hdr * eth_hdr =rte_pktmbuf_mtod(mbuf,struct ether_hdr*);
	struct mpls_hdr  * mpls_hdr=(struct mpls_hdr*)(eth_hdr+1);
	struct pbp_cache_entry * cache=NULL;
	if(PREDICT_FALSE(eth_hdr->ether_type!=ETHER_PROTO_MPLS_UNICAST)){
		/*
		*any non-MPLS packet goes to host stack
		*/
		fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_HOST_STACK,0);
		goto ret;
	}
	label=mpls_label(mpls_hdr);
	if(PREDICT_FALSE(!mpls_ttl(mpls_hdr))){
		/*discard any packet with TTL equal to 0*/
		goto ret;
	}
	
	cache_index=label&PBP_CACHE_MASK;
	cache=&pbp_cache[cache_index];
	{
		/*
		*process pbp cache relavant affairs.
		*/
		if(PREDICT_FALSE(cache->label!=label)){
			cache->is_valid=0;
			cache->label=label;
			cache->lentry=spine_label_entry_at(priv->label_base,label);
			if(!cache->lentry->is_valid)
				goto normal;
			if(cache->lentry->is_unicast){
				cache->unicast_nexthop=
					find_common_nexthop(cache->lentry->NHLFE);
				if((!(cache->unicast_nexthop))||(!(cache->unicast_nexthop->is_valid)))
					goto normal;
				cache->unicast_neighbor=
					find_common_neighbor(cache->unicast_nexthop->common_neighbor_index);
				if((!(cache->unicast_neighbor))||(!(cache->unicast_neighbor->is_valid)))
					goto normal;
			}else{
				cache->multicast_nexthops=
					find_mnext_hops(cache->lentry->NHLFE);
				if((!(cache->multicast_nexthops))||(!(cache->multicast_nexthops->is_valid)))
					goto normal;
			}
			cache->is_valid=1;
		}
		normal:
		/*
		*drop any packet whose fwd entries are incomplete
		*/
		if(PREDICT_FALSE(!cache->is_valid)){
			fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_DROP,0);
			goto ret;
		}
	}
	/*until here we are sure the fwd credentials are ready*/
	if(PREDICT_TRUE(cache->lentry->is_unicast)){
		
		/*unicast mpls packet:rewrite the outer MPLS header*/
		set_mpls_label(mpls_hdr,cache->lentry->swapped_label);
		set_mpls_ttl(mpls_hdr,mpls_ttl(mpls_hdr)-1);
		if(PREDICT_FALSE(!(psrc_iface=
			find_e3interface_by_index(cache->unicast_nexthop->local_e3iface)))){
			fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_DROP,0);
			goto ret;
		}
		rte_memcpy(eth_hdr->d_addr.addr_bytes,
				cache->unicast_neighbor->mac,
				6);
		rte_memcpy(eth_hdr->s_addr.addr_bytes,
				psrc_iface->mac_addrs,
				6);
		fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_UNICAST_FWD,
			cache->unicast_nexthop->local_e3iface);
	}else{
		fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_MULTICAST_FWD,label);
	}
	ret:
	return fwd_id;	
}

inline int _pbp_multicast_fordward_slow_path(struct E3Interface *pif,
						struct node * pnode,
						struct rte_mbuf **mbufs,
						int nr_mbufs,
						int label)
{
	struct pbp_private * priv=(struct pbp_private*)pif->private;
	struct spine_label_entry * lentry=spine_label_entry_at(priv->label_base,label);
	struct multicast_next_hops * pmnexthops=find_mnext_hops(lentry->NHLFE);
	struct common_nexthop * pnext;
	struct common_neighbor *pneighbor;
	struct E3Interface *psrc_if;
	int idx_packet;
	int idx_hop;
	int iptr;
	int consumed;
	int mbuf_ready;
	int nr_delivered;
	int numa_socket=lcore_to_socket_id(pnode->lcore_id);
	struct rte_mempool * mempool=get_mempool_by_socket_id(numa_socket<0?0:numa_socket);
	
	nr_mbufs=nr_mbufs>PBP_NODE_BURST_SIZE?PBP_NODE_BURST_SIZE:nr_mbufs;
	struct rte_mbuf * mbufs_invt[MAX_HOPS_IN_MULTICAST_GROUP][PBP_NODE_BURST_SIZE];
	int iptr_invt[MAX_HOPS_IN_MULTICAST_GROUP];/*store the exact number of packet horizontally*/
	memset(iptr_invt,0x0,sizeof(iptr_invt));

	for(idx_packet=0;idx_packet<nr_mbufs;idx_packet++){
		consumed=0;
		/*
		*idx_hop is the indicator which shows how many valid nexthops are proceeded
		*while iptr is the actual index which shows the actual nexthop location
		*/
		for(idx_hop=0,iptr=0;
			(idx_hop<pmnexthops->nr_hops)&&(iptr<MAX_HOPS_IN_MULTICAST_GROUP);
			iptr++){
			/*
			*skip all the invalid nexthop
			*/
			if(!pmnexthops->nexthops[iptr].is_valid)
				continue;
			/*already found one, no matter what, preceed one step further
			*this will accelerate the procedure of multicast forwarding
			*/
			idx_hop++;
			pnext=find_common_nexthop(pmnexthops->nexthops[iptr].next_hop);
			if((!pnext)||
				(!(pneighbor=find_common_neighbor(pnext->common_neighbor_index)))||
				(!(psrc_if=find_e3interface_by_index(pnext->local_e3iface))))
				continue;
			/*
			*perform Rrverse Path Check
			*/
			if(pmnexthops->nexthops[iptr].label_to_push==lentry->swapped_label)
				continue;
			mbuf_ready=0;
			/*
			*it seldom happens that nexthop entry in
			*mnexthops sets. 
			*/
			if(idx_hop==pmnexthops->nr_hops){
				mbufs_invt[iptr][iptr_invt[iptr]]=mbufs[idx_packet];
				consumed=1;
				mbuf_ready=1;
			}else{
				mbufs_invt[iptr][iptr_invt[iptr]]=rte_pktmbuf_alloc(mempool);
				if(mbufs_invt[iptr][iptr_invt[iptr]]){
					rte_pktmbuf_append(mbufs_invt[iptr][iptr_invt[iptr]],
						mbufs[idx_packet]->pkt_len);
					rte_memcpy(rte_pktmbuf_mtod(mbufs_invt[iptr][iptr_invt[iptr]],void*),
						rte_pktmbuf_mtod(mbufs[idx_packet],void*),
						mbufs[idx_packet]->pkt_len);
					mbuf_ready=1;
				}
			}
			if(PREDICT_TRUE(mbuf_ready)){
				struct ether_hdr * eth_hdr=rte_pktmbuf_mtod(mbufs_invt[iptr][iptr_invt[iptr]],struct ether_hdr*);
				struct mpls_hdr  * mpls=(struct mpls_hdr *)(eth_hdr+1);
				set_mpls_label(mpls,pmnexthops->nexthops[iptr].label_to_push);
				set_mpls_ttl(mpls,mpls_ttl(mpls)-1);
				rte_memcpy(eth_hdr->d_addr.addr_bytes,
					pneighbor->mac,
					6);
				rte_memcpy(eth_hdr->s_addr.addr_bytes,
					psrc_if->mac_addrs,
					6);
				iptr_invt[iptr]++;
			}
		}
		if(PREDICT_FALSE(!consumed))
			rte_pktmbuf_free(mbufs[idx_packet]);
	}
	
	for(iptr=0;iptr<MAX_HOPS_IN_MULTICAST_GROUP;iptr++){
		nr_delivered=0;
		if(!iptr_invt[iptr])
			continue;
		pnext=find_common_nexthop(pmnexthops->nexthops[iptr].next_hop);
		psrc_if=find_e3interface_by_index(pnext->local_e3iface);
		nr_delivered=deliver_mbufs_to_e3iface(psrc_if,
			0,
			mbufs_invt[iptr],
			iptr_invt[iptr]);
		
		for(idx_packet=nr_delivered;idx_packet<iptr_invt[iptr];idx_packet++)
			rte_pktmbuf_free(mbufs_invt[iptr][idx_packet]);
		
	}
	return nr_mbufs;
}
inline void _post_pbp_input_packet_process(struct E3Interface * pif,
								struct node * pnode,
								struct rte_mbuf **mbufs,
								int nr_mbufs,
								uint64_t fwd_id)
{
	int idx=0;
	int drop_start=0;
	int nr_dropped=0;
	int nr_delivered=0;
	
	switch(HIGH_UINT64(fwd_id))
	{
		case PBP_PROCESS_INPUT_HOST_STACK:
			{
				struct E3Interface * ppeer=NULL;
				if(PREDICT_FALSE((!pif->has_peer_device)||
					!(ppeer=find_e3interface_by_index(pif->peer_port_id)))){
					nr_dropped=nr_mbufs;
					break;
				}
				nr_delivered=deliver_mbufs_to_e3iface(ppeer,
					0,
					mbufs,
					nr_mbufs);
				drop_start=nr_delivered;
				nr_dropped=nr_mbufs-drop_start;
			}
				
			break;
		case PBP_PROCESS_INPUT_UNICAST_FWD:
			{
				uint32_t dst_iface=LOW_UINT32(fwd_id);
				struct E3Interface * pdst=NULL;
				if(PREDICT_FALSE(!(pdst=find_e3interface_by_index(dst_iface)))){
					nr_dropped=nr_mbufs;
					break;
				}
				nr_delivered=deliver_mbufs_to_e3iface(pdst,
					0,
					mbufs,
					nr_mbufs);
				drop_start=nr_delivered;
				nr_dropped=nr_mbufs-drop_start;
			}
			break;
		case PBP_PROCESS_INPUT_MULTICAST_FWD:
			{
				uint32_t label=LOW_UINT64(fwd_id);
				nr_delivered=_pbp_multicast_fordward_slow_path(pif,
					pnode,
					mbufs,
					nr_mbufs,
					label);
				drop_start=nr_delivered;
				nr_dropped=nr_mbufs-drop_start;
			}
			break;
		default:
			nr_dropped=nr_mbufs;
			break;
	}
	for(idx=0;idx<nr_dropped;idx++)
		rte_pktmbuf_free(mbufs[drop_start+idx]);
}
int provider_backbone_port_iface_input_iface(void * arg)
{
	struct rte_mbuf *    mbufs[PBP_NODE_BURST_SIZE];
	int                  nr_rx;
	int                  iptr;
	int 				 process_rc;
	int                  start_index,
					     end_index;
	struct node *        pnode=(struct node *)arg;
	int                  iface=HIGH_UINT64((uint64_t)pnode->node_priv);
	int                  queue_id=LOW_UINT64((uint64_t)pnode->node_priv);
	struct E3Interface * pif=find_e3interface_by_index(iface);
	struct pbp_private * priv=NULL;
	DEF_EXPRESS_DELIVERY_VARS();
	RESET_EXPRESS_DELIVERY_VARS();
	uint64_t fwd_id;
	uint64_t last_fwd_id;
	struct pbp_cache_entry pbp_cache[PBP_CACHE_SIZE];
	memset(pbp_cache,0x0,sizeof(pbp_cache));
	if(PREDICT_FALSE(!pif))
		return 0;
	priv=(struct pbp_private*)pif->private;
	nr_rx=rte_eth_rx_burst(iface,queue_id,mbufs,PBP_NODE_BURST_SIZE);
	pre_setup_env(nr_rx);
	while((iptr=peek_next_mbuf())>=0){
		prefetch_next_mbuf(mbufs,iptr);
		fwd_id=_process_pbp_input_packet(mbufs[iptr],pbp_cache,priv);
		process_rc=proceed_mbuf(iptr,fwd_id);
		if(process_rc==MBUF_PROCESS_RESTART){
			fetch_pending_index(start_index,end_index);
			fetch_pending_fwd_id(last_fwd_id);
			_post_pbp_input_packet_process(pif,
						pnode,
						&mbufs[start_index],
						end_index-start_index+1,
						last_fwd_id);
			flush_pending_mbuf();
			proceed_mbuf(iptr,fwd_id);
		}
	}
	fetch_pending_index(start_index,end_index);
	fetch_pending_fwd_id(last_fwd_id);
	if(PREDICT_TRUE(start_index>=0))
		_post_pbp_input_packet_process(pif,
						pnode,
						&mbufs[start_index],
						end_index-start_index+1,
						last_fwd_id);
	return 0;
}

int provider_backbone_port_iface_output_iface(void * arg)
{
	int idx=0;
	struct rte_mbuf *mbufs[32];
	int nr_rx;
	int nr_tx;
	struct node * pnode=(struct node *)arg;
	int iface_id=HIGH_UINT64((uint64_t)pnode->node_priv);
	int queue_id=LOW_UINT64((uint64_t)pnode->node_priv);
	struct E3Interface * pif=NULL;
	if(!(pif=find_e3interface_by_index(iface_id)))/*this never happens*/
		return -1;
	if(!(nr_rx=rte_ring_sc_dequeue_burst(pnode->node_ring,(void**)mbufs,32,NULL)))
		return 0;
	nr_tx=rte_eth_tx_burst(iface_id,queue_id,mbufs,nr_rx);
	for(idx=nr_tx;idx<nr_rx;idx++)
		rte_pktmbuf_free(mbufs[idx]);
	return 0;
}

int provider_backbone_port_iface_post_setup(struct E3Interface * pif)
{
	struct pbp_private * priv=(struct pbp_private*)pif->private;
	pif->hwiface_role=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT;
	/*setup the label base for the e3interface*/
	rte_rwlock_init(&priv->pbp_guard);
	priv->label_base=allocate_label_entry_base(-1);
	if(!priv->label_base){
		E3_ERROR("can not allocate label entry base for provider backbone port\n");
		return -1;
	}
	E3_LOG("back E3Interface %d as provider backbone port\n",pif->port_id);
	return 0;
}
int provider_backbone_port_iface_delete(struct E3Interface * pif)
{
	/*
	*withdraw all the label of the ports
	*/
	int idx=0;
	struct pbp_private * priv=(struct pbp_private*)pif->private;
	E3_ASSERT(pif->hwiface_role==E3IFACE_ROLE_PROVIDER_BACKBONE_PORT);
	for(idx=0;idx<NR_SPINE_LABEL_ENTRY;idx++){
		if(priv->label_base[idx].is_valid){
			reset_spine_label_entry(priv->label_base,idx);
		}
	}
	RTE_FREE(priv->label_base);
	priv->label_base=NULL;
	E3_LOG("successfully reclaim provider backbone port %d's label entry base\n",pif->port_id);
	E3_LOG("release provider backbone port %d's private data\n",pif->port_id);
	return 0;
}

void pbp_init(void)
{
	
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].is_set=1;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].capability_check=null_capability_check;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].priv_size=sizeof(struct pbp_private);
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].input_node_process_func=provider_backbone_port_iface_input_iface;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].output_node_process_func=provider_backbone_port_iface_output_iface;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].post_setup=provider_backbone_port_iface_post_setup;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].iface_delete=provider_backbone_port_iface_delete;
}
E3_init(pbp_init,(TASK_PTIORITY_LOW+1));

