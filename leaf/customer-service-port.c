/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#define __CONTEXT_CUSTOEMR_SERVICE_PORT

#include <leaf/include/customer-service-port.h>
#include <e3_init.h>
#include <e3iface-inventory.h>
#include <leaf/include/leaf-e-service.h>
#include <leaf/include/leaf-label-fib.h>
#include <util.h>
#include <e3net/include/mpls-util.h>
#include <node.h>
#include <mbuf_delivery.h>
#include <e3net/include/mpls-util.h>
#include <rte_ether.h>
#include <e3net/include/common-cache.h>
#include <lcore_extension.h>
extern struct e3iface_role_def  role_defs[E3IFACE_ROLE_MAX_ROLES];
#define CSP_NODE_BURST_SIZE 48

#define CSP_MAC_CACHE_SIZE 8
#define CSP_MAC_CACHE_MASK (CSP_MAC_CACHE_SIZE-1)


#define CSP_PROCESS_INPUT_DROP 0x0
#define CSP_PROCESS_INPUT_ELINE_FWD 0x1
#define CSP_PROCESS_INPUT_ELAN_UNICAST_FWD 0x2
#define CSP_PROCESS_INPUT_ELAN_MULTICAST_FWD 0x3
//#define CSP_PROCESS_INPUT_HOST_STACK 0x4


static int csp_capability_check(int port_id)
{
	struct rte_eth_dev_info dev_info;
	rte_eth_dev_info_get(port_id,&dev_info);
	if(!(dev_info.rx_offload_capa&DEV_RX_OFFLOAD_VLAN_STRIP)||
		!(dev_info.tx_offload_capa&DEV_TX_OFFLOAD_VLAN_INSERT))
		return -1;
	return 0;
}
inline uint64_t _csp_process_input_packet(struct rte_mbuf*mbuf,
	struct csp_cache_entry *csp_cache,
	struct mac_cache_entry *mac_cache,
	struct csp_private * priv)
{
	uint64_t fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_DROP,0);	
	struct ether_hdr *eth_hdr=rte_pktmbuf_mtod(mbuf,struct ether_hdr*);
	uint16_t pkt_vlan_tci=0;
	uint16_t mcache_index;
	struct mac_cache_entry * mcache;
	if(mbuf->ol_flags&PKT_RX_VLAN_STRIPPED){
		pkt_vlan_tci=mbuf->vlan_tci;
	}
	/*
	*vlan must be the same as the priv's vlan_tci
	*/
	if(PREDICT_FALSE(priv->vlan_tci!=pkt_vlan_tci))
		goto ret;
	/*
	*process csp_cache
	*/
	if(PREDICT_FALSE(csp_cache->vlan_tci!=pkt_vlan_tci)){
		csp_cache->vlan_tci=pkt_vlan_tci;
		csp_cache->is_valid=0;
		if(!priv->attached)
			goto continue_go;
		switch(priv->e_service)
		{
			case e_line_service:
				csp_cache->is_e_line=1;
				if((!(csp_cache->eline=find_e_line_service(priv->service_index)))||
					(!(csp_cache->eline_nexthop=find_common_nexthop(csp_cache->eline->NHLFE)))||
					(!(csp_cache->eline_src_pif=find_e3interface_by_index(csp_cache->eline_nexthop->local_e3iface)))||
					(!(csp_cache->eline_neighbor=find_common_neighbor(csp_cache->eline_nexthop->common_neighbor_index))))
					goto continue_go;
				break;
			case e_lan_service:
				csp_cache->is_e_line=0;
				if(!(csp_cache->elan=find_e_lan_service(priv->service_index)))
					goto continue_go;
				break;
			default:
				goto continue_go;
				break;
		}
		csp_cache->is_valid=1;
	}
	continue_go:
	if(PREDICT_FALSE(!csp_cache->is_valid))
		goto ret;
	if(csp_cache->is_e_line){
		/*
		*here it can crash if no room to prepend.
		*increase the mbuf's header room space could resolve this issue
		*/
		struct ether_hdr* outer_eth_hdr=(struct ether_hdr*)rte_pktmbuf_prepend(mbuf,18);
		struct mpls_hdr * outer_mpls_hdr=(struct mpls_hdr*)outer_eth_hdr;
		rte_memcpy(outer_eth_hdr->d_addr.addr_bytes,
			csp_cache->eline_neighbor->mac,
			6);
		rte_memcpy(outer_eth_hdr->s_addr.addr_bytes,
			csp_cache->eline_src_pif->mac_addrs,
			6);
		outer_eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;

		set_mpls_bottom(outer_mpls_hdr);
		set_mpls_exp(outer_mpls_hdr,0);
		set_mpls_ttl(outer_mpls_hdr,0x40);
		set_mpls_label(outer_mpls_hdr,csp_cache->eline->label_to_push);
		fwd_id=MAKE_UINT64(ETHER_PROTO_MPLS_UNICAST,csp_cache->eline_nexthop->local_e3iface);
	}else{
		mcache_index=eth_hdr->d_addr.addr_bytes[5];
		mcache_index&=CSP_MAC_CACHE_MASK;
		mcache=&mac_cache[mcache_index];

		if(PREDICT_FALSE(!IS_MAC_EQUAL(mcache->mac,eth_hdr->d_addr.addr_bytes))){
			struct findex_2_4_key key={
				.value_as_u64=0,
			};
			mcache->is_valid=0;
			rte_memcpy(mcache->mac,eth_hdr->d_addr.addr_bytes,6);
			mac_to_findex_2_4_key(mcache->mac,&key);
			if(!fast_index_2_4_item_safe(csp_cache->elan->fib_base,&key)){
				mcache->fwd_entry.entry_as_u64=key.value_as_u64;
				mcache->is_valid=1;
				if((!mcache->fwd_entry.is_port_entry)&&(
					(!(mcache->nexthop=find_common_nexthop(mcache->fwd_entry.NHLFE)))||
					(!(mcache->neighbor=find_common_neighbor(mcache->nexthop->common_neighbor_index)))||
					(!(mcache->src_pif=find_e3interface_by_index(mcache->nexthop->local_e3iface))))){
					mcache->is_valid=0;
				}	
			}
		}
		if(PREDICT_TRUE(mcache->is_valid)){
			if(!mcache->fwd_entry.is_port_entry){
				/*
				*this fwd entry is to fwd a packet into MPLS network with determined path
				*let's do encapsulation first
				*/
				struct ether_hdr* outer_eth_hdr=(struct ether_hdr*)rte_pktmbuf_prepend(mbuf,18);
				struct mpls_hdr * outer_mpls_hdr=(struct mpls_hdr*)outer_eth_hdr;

				rte_memcpy(outer_eth_hdr->d_addr.addr_bytes,
					mcache->neighbor->mac,
					6);
				rte_memcpy(outer_eth_hdr->s_addr.addr_bytes,
					mcache->src_pif->mac_addrs,
					6);
				outer_eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
				set_mpls_bottom(outer_mpls_hdr);
				set_mpls_exp(outer_mpls_hdr,0x0);
				set_mpls_ttl(outer_mpls_hdr,0x40);
				set_mpls_label(outer_mpls_hdr,mcache->fwd_entry.label_to_push);
				fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_ELAN_UNICAST_FWD,mcache->nexthop->local_e3iface);
			}else{
				if(mcache->fwd_entry.vlan_tci){
					mbuf->vlan_tci=mcache->fwd_entry.vlan_tci;
					mbuf->ol_flags=PKT_TX_VLAN_PKT;
				}
				fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_ELAN_UNICAST_FWD,mcache->fwd_entry.e3iface);
			}
		}else{
			fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_ELAN_MULTICAST_FWD,0);
		}
	}
	ret:
	return fwd_id;
}
inline int _csp_multicast_forward_slow_path(struct E3Interface *pif,
		struct node * pnode,
		struct rte_mbuf **mbufs,
		int nr_mbufs)
{
	struct rte_mempool * mempool=get_mempool_by_socket_id(lcore_to_socket_id(pnode->lcore_id));
	struct csp_private *priv=(struct csp_private*)pif->private;
	int nr_ports=0;
	int nr_port=0;
	int port_id=0;
	int iptr;
	
	struct ether_e_lan *elan=NULL;
	struct rte_mbuf *mbufs_to_send[CSP_NODE_BURST_SIZE];
	int nr_send=0;
	int nr_tx;
	struct E3Interface * pif_dst;

	struct common_nexthop  * mc_nexthop=NULL;
	struct common_neighbor * mc_neighbor=NULL;
	struct E3Interface     * mc_src_if=NULL;
	
	if(PREDICT_FALSE((!priv->attached)||
		(priv->e_service!=e_lan_service)||
		(!(elan=find_e_lan_service(priv->service_index)))))
		return 0;
	nr_ports=elan->nr_ports;

	for(port_id=0;(port_id<MAX_PORTS_IN_E_LAN_SERVICE)&&(nr_port<nr_ports);port_id++){
		if(!elan->ports[port_id].is_valid)
			continue;
		pif_dst=find_e3interface_by_index(elan->ports[port_id].iface);
		if(!pif_dst){
			nr_port++;
			continue;
		}
		/*
		*prepare packet for this interface
		*/
		for(nr_send=0,iptr=0;iptr<nr_mbufs;iptr++){
			mbufs_to_send[nr_send]=rte_pktmbuf_alloc(mempool);
			if(!mbufs_to_send[nr_send])
				break;
			rte_pktmbuf_append(mbufs_to_send[nr_send],mbufs[iptr]->pkt_len);
			rte_memcpy(rte_pktmbuf_mtod(mbufs_to_send[nr_send],void*),
						rte_pktmbuf_mtod(mbufs[iptr],void*),
						mbufs[iptr]->pkt_len);
			if(elan->ports[port_id].vlan_tci){
				mbufs_to_send[nr_send]->vlan_tci=elan->ports[port_id].vlan_tci;
				mbufs_to_send[nr_send]->ol_flags=PKT_TX_VLAN_PKT;
			}
			nr_send++;
		}
		nr_tx=deliver_mbufs_to_e3iface(pif_dst,0,mbufs_to_send,nr_send);
		for(iptr=nr_tx;iptr<nr_send;iptr++)
			rte_pktmbuf_free(mbufs_to_send[iptr]);
		nr_port++;
	}
	/*
	*prepare a copy to multicat path of e-lan
	*/
	if((!(mc_nexthop=find_common_nexthop(elan->multicast_NHLFE)))||
		(!(mc_src_if=find_e3interface_by_index(mc_nexthop->local_e3iface)))||
		(!(mc_neighbor=find_common_neighbor(mc_nexthop->common_neighbor_index))))
		return 0;
	for(iptr=0;iptr<nr_mbufs;iptr++){
		struct ether_hdr * outer_eth_hdr=(struct ether_hdr*)rte_pktmbuf_prepend(mbufs[iptr],18);
		struct mpls_hdr  * outer_mpls_hdr=(struct mpls_hdr*)(outer_eth_hdr+1);
		rte_memcpy(outer_eth_hdr->d_addr.addr_bytes,
			mc_neighbor->mac,
			6);
		rte_memcpy(outer_eth_hdr->s_addr.addr_bytes,
			mc_src_if->mac_addrs,
			6);
		outer_eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;

		set_mpls_bottom(outer_mpls_hdr);
		set_mpls_exp(outer_mpls_hdr,0x0);
		set_mpls_ttl(outer_mpls_hdr,0x40);
		set_mpls_label(outer_mpls_hdr,elan->multicast_label);
	}
	nr_tx=deliver_mbufs_to_e3iface(mc_src_if,
		0,
		mbufs,
		nr_mbufs);
	return nr_tx;
}
inline void _post_csp_input_packet_process(struct E3Interface*pif,
		struct node * pnode,
		struct rte_mbuf **mbufs,
		int nr_mbufs,
		uint64_t fwd_id)
{
	int idx=0;
	int drop_start=0;
	int nr_dropped=nr_mbufs;
	int nr_deliveded=0;
	switch(HIGH_UINT64(fwd_id))
	{
		case CSP_PROCESS_INPUT_ELINE_FWD:
		case CSP_PROCESS_INPUT_ELAN_UNICAST_FWD:
			{
				uint32_t dst_iface=LOW_UINT64(fwd_id);
				struct E3Interface *pdst=NULL;
				if(PREDICT_FALSE(!(pdst=find_e3interface_by_index(dst_iface)))){
					nr_dropped=nr_mbufs;
					break;
				}
				nr_deliveded=deliver_mbufs_to_e3iface(pdst,
					0,
					mbufs,
					nr_mbufs);
				drop_start=nr_deliveded;
				nr_dropped=nr_mbufs-drop_start;
			}
			break;
		case CSP_PROCESS_INPUT_ELAN_MULTICAST_FWD:
			nr_deliveded=_csp_multicast_forward_slow_path(pif,
				pnode,
				mbufs,
				nr_mbufs);
			drop_start=nr_deliveded;
			nr_dropped=nr_mbufs-drop_start;
			break;
		default:
			nr_dropped=nr_mbufs;
			break;
	}
	for(idx=0;idx<nr_dropped;idx++)
		rte_pktmbuf_free(mbufs[idx+drop_start]);
}

int customer_service_port_iface_input_iface(void * arg)
{
	struct rte_mbuf * 	mbufs[CSP_NODE_BURST_SIZE];
	int 				nr_rx;
	int 				iptr;
	int 				process_rc;
	int					start_index;
	int					end_index;
	struct node *		pnode=(struct node*)arg;
	int 				iface=HIGH_UINT64((uint64_t)pnode->node_priv);
	int					queue_id=LOW_UINT64((uint64_t)pnode->node_priv);
	uint64_t 			fwd_id;
	uint64_t 			last_fwd_id;
	struct E3Interface*	pif=find_e3interface_by_index(iface);
	struct csp_private*	priv=NULL;
	struct csp_cache_entry csp_cache={
		.vlan_tci=-1,
		.is_valid=0,
	};
	struct mac_cache_entry mac_cache[CSP_MAC_CACHE_SIZE];
	memset(mac_cache,0x0,sizeof(mac_cache));
	
	DEF_EXPRESS_DELIVERY_VARS();
	RESET_EXPRESS_DELIVERY_VARS();
	if(PREDICT_FALSE(!pif))
		return 0;
	priv=(struct csp_private*)pif->private;
	if(PREDICT_FALSE(!priv->attached))
		return 0;
	
	
	nr_rx=rte_eth_rx_burst(iface,queue_id,mbufs,CSP_NODE_BURST_SIZE);
	pre_setup_env(nr_rx);
	while((iptr=peek_next_mbuf())>=0){
		prefetch_next_mbuf(mbufs,iptr);
		fwd_id=_csp_process_input_packet(mbufs[iptr],&csp_cache,mac_cache,priv);
		process_rc=proceed_mbuf(iptr,fwd_id);
		if(process_rc==MBUF_PROCESS_RESTART){
			fetch_pending_index(start_index,end_index);
			fetch_pending_fwd_id(last_fwd_id);
			_post_csp_input_packet_process(pif,
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
		_post_csp_input_packet_process(pif,
				pnode,
				&mbufs[start_index],
				end_index-start_index+1,
				last_fwd_id);
	return 0;
}
int customer_service_port_iface_output_iface(void * arg)
{
	int idx=0;
	struct rte_mbuf *mbufs[32];
	int nr_rx;
	int nr_tx;
	struct node * pnode=(struct node *)arg;
	int iface_id=HIGH_UINT64((uint64_t)pnode->node_priv);
	int queue_id=LOW_UINT64((uint64_t)pnode->node_priv);
	struct E3Interface * pif=NULL;
	if(!(pif=find_e3interface_by_index(iface_id)))
		return -1;
	if(!(nr_rx=rte_ring_sc_dequeue_burst(pnode->node_ring,(void**)mbufs,32,NULL)))
		return 0;
	nr_tx=rte_eth_tx_burst(iface_id,queue_id,mbufs,nr_rx);
	for(idx=nr_tx;idx<nr_rx;idx++)
		rte_pktmbuf_free(mbufs[idx]);
	return 0;
}
int customer_service_port_iface_post_setup(struct E3Interface * pif)
{
	struct csp_private * priv=(struct csp_private*)pif->private;
	pif->hwiface_role=E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT;
	memset(priv,0x0,sizeof(struct csp_private));
	/*setup vlan stripping here*/
	return 0;
}
int customer_service_port_iface_delete(struct E3Interface * pif)
{
	struct csp_private * priv=(struct csp_private*)pif->private;
	if(priv->attached&&(priv->e_service==e_lan_service)){
		int elan_port_id=find_e_lan_port(priv->service_index,pif->port_id,priv->vlan_tci);
		if(elan_port_id>=0){
			delete_e_lan_port(priv->service_index,elan_port_id);
		}
	}	
	return 0;
}

void csp_init(void)
{
	role_defs[E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT].is_set=1;
	role_defs[E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT].capability_check=csp_capability_check;
	role_defs[E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT].priv_size=sizeof(struct csp_private);
	role_defs[E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT].input_node_process_func=customer_service_port_iface_input_iface;
	role_defs[E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT].output_node_process_func=customer_service_port_iface_output_iface;
	role_defs[E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT].post_setup=customer_service_port_iface_post_setup;
	role_defs[E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT].iface_delete=customer_service_port_iface_delete;

}

E3_init(csp_init,(TASK_PTIORITY_LOW+1));

