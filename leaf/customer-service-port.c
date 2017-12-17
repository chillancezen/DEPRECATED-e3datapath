/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#define __CONTEXT_CUSTOEMR_SERVICE_PORT

#include <leaf/include/customer-service-port.h>
#include <e3infra/include/e3-init.h>
#include <e3net/include/e3iface-inventory.h>
#include <leaf/include/leaf-e-service.h>
#include <leaf/include/leaf-label-fib.h>
#include <e3infra/include/util.h>
#include <e3net/include/mpls-util.h>
#include <e3infra/include/node.h>
#include <e3infra/include/mbuf-delivery.h>
#include <e3net/include/mpls-util.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <e3net/include/common-cache.h>
#include <e3infra/include/lcore-extension.h>
#include <leaf/include/mac-learning.h>
#include <rte_cycles.h>

extern struct e3iface_role_def  role_defs[E3IFACE_ROLE_MAX_ROLES];

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
	struct csp_cache_entry *_csp_cache,
	struct mac_cache_entry *_mac_cache,
	struct mac_learning_cache_entry *mac_learning_cache,
	int	*  nr_mac_learning_cache,
	int e3iface,
	struct csp_private * priv)
{
	uint64_t fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_DROP,0);	
	struct ether_hdr *eth_hdr=rte_pktmbuf_mtod(mbuf,struct ether_hdr*);
	uint16_t pkt_vlan_tci=0;
	uint16_t vlan_index=0;
	uint16_t mcache_index;
	uint16_t idx=0;
	struct csp_cache_entry * vcache;
	struct mac_cache_entry * mcache;
	if(mbuf->ol_flags&PKT_RX_VLAN_STRIPPED){
		pkt_vlan_tci=mbuf->vlan_tci;
	}
	vlan_index=pkt_vlan_tci&CSP_CACHE_MASK;
	
	/*
	*process csp_cache, cache vlan entry,
	*if cache hits, next time accessing priv vlan distribution
	*will be skipped.
	*/
	vcache=&_csp_cache[vlan_index];
	if(PREDICT_FALSE(vcache->vlan_tci!=pkt_vlan_tci)){
		vcache->vlan_tci=pkt_vlan_tci;
		vcache->is_valid=0;
		/*
		*vlan-id must be enabled in priv VLAN distribution table
		*/
		if(!priv->vlans[pkt_vlan_tci].is_valid)
			goto continue_go;
		switch(priv->vlans[pkt_vlan_tci].e_service)
		{
			case e_line_service:
				vcache->is_e_line=1;
				if((!(vcache->eline=find_e_line_service(priv->vlans[pkt_vlan_tci].service_index)))||
					(!vcache->eline->is_cbp_ready)||/*let it be detected earlier if CBP not ready*/
					(!(vcache->eline_nexthop=find_common_nexthop(vcache->eline->NHLFE)))||
					(!(vcache->eline_src_pif=find_e3interface_by_index(vcache->eline_nexthop->local_e3iface)))||
					(!(vcache->eline_neighbor=find_common_neighbor(vcache->eline_nexthop->common_neighbor_index))))
					goto continue_go;
				break;
			case e_lan_service:
				vcache->is_e_line=0;
				if(!(vcache->elan=find_e_lan_service(priv->vlans[pkt_vlan_tci].service_index)))
					goto continue_go;
				break;
			default:
				goto continue_go;
				break;
		}
		vcache->is_valid=1;
	}
	continue_go:
	
	if(PREDICT_FALSE(!vcache->is_valid))
		goto ret;
	if(vcache->is_e_line){
		/*
		*here it can crash if no room to prepend.
		*increase the mbuf's header room space could resolve this issue
		*/
		struct ether_hdr* outer_eth_hdr=(struct ether_hdr*)rte_pktmbuf_prepend(mbuf,18);
		struct mpls_hdr * outer_mpls_hdr=(struct mpls_hdr*)(outer_eth_hdr+1);
		rte_memcpy(outer_eth_hdr->d_addr.addr_bytes,
			vcache->eline_neighbor->mac,
			6);
		rte_memcpy(outer_eth_hdr->s_addr.addr_bytes,
			vcache->eline_src_pif->mac_addrs,
			6);
		outer_eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;

		set_mpls_bottom(outer_mpls_hdr);
		set_mpls_exp(outer_mpls_hdr,0);
		set_mpls_ttl(outer_mpls_hdr,0x40);
		set_mpls_label(outer_mpls_hdr,vcache->eline->label_to_push);
		fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_ELINE_FWD,vcache->eline_nexthop->local_e3iface);
	}else{
		/*
		*enter E-LAN logics,do mac based forwarding,
		*and snoop for mac learning
		*/
		mcache_index=eth_hdr->d_addr.addr_bytes[5];
		mcache_index&=CSP_MAC_CACHE_MASK;
		mcache=&_mac_cache[mcache_index];

		if(PREDICT_FALSE(!IS_MAC_EQUAL(mcache->mac,eth_hdr->d_addr.addr_bytes))){
			struct findex_2_4_key key={
				.leaf_fwd_entry_as_64=0,
			};
			mcache->is_valid=0;
			rte_memcpy(mcache->mac,eth_hdr->d_addr.addr_bytes,6);
			mac_to_findex_2_4_key(mcache->mac,&key);
			if(!fast_index_2_4_item_safe(vcache->elan->fib_base,&key)){
				mcache->fwd_entry.entry_as_u64=key.leaf_fwd_entry_as_64;
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
				/*todo:check source port+vlan to prohibit hairpin forwarding*/
				struct ether_hdr* outer_eth_hdr=(struct ether_hdr*)rte_pktmbuf_prepend(mbuf,18);
				struct mpls_hdr * outer_mpls_hdr=(struct mpls_hdr*)(outer_eth_hdr+1);

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
			fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_ELAN_MULTICAST_FWD,pkt_vlan_tci);
		}
		/*
		*inspect the inner ether header,and learn mac and port+vlan mapping
		*in E-LAN's fib base,first make sure the customer source mac is legal unicast
		* address
		*/
		if(PREDICT_TRUE(!(eth_hdr->s_addr.addr_bytes[0]&0x1))){
			for(idx=0;idx<*nr_mac_learning_cache;idx++){
				if(IS_MAC_EQUAL(eth_hdr->s_addr.addr_bytes,mac_learning_cache[idx].mac))
					break;
			}
			if(idx==*nr_mac_learning_cache){
				struct e_lan_fwd_entry _fwd_entry={
					.is_port_entry=1,
					.e3iface=e3iface,
					.vlan_tci=pkt_vlan_tci,
				};
				copy_ether_address(mac_learning_cache[idx].mac,
					eth_hdr->s_addr.addr_bytes);
				mac_learning_cache[idx].elan_index=vcache->elan->index;
				mac_learning_cache[idx].fwd_entry_as64=_fwd_entry.entry_as_u64;
				(*nr_mac_learning_cache)++;
			}
		}
	}
	ret:
	return fwd_id;
}
inline int _csp_multicast_forward_slow_path(struct E3Interface *pif,
		struct node * pnode,
		struct rte_mbuf **mbufs,
		int nr_mbufs,
		uint32_t vlan_tci)
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
	
	if(PREDICT_FALSE((!priv->vlans[vlan_tci].is_valid)||
		(priv->vlans[vlan_tci].e_service!=e_lan_service)||
		(!(elan=find_e_lan_service(priv->vlans[vlan_tci].service_index)))))
		return 0;
	nr_ports=elan->nr_ports;

	for(port_id=0;(port_id<MAX_PORTS_IN_E_LAN_SERVICE)&&(nr_port<nr_ports);port_id++){
		if(!elan->ports[port_id].is_valid)
			continue;
		/*
		*skip the port which is regarded as 
		*input port or dst port not found
		*/
		if(((elan->ports[port_id].iface==pif->port_id)&&
			(elan->ports[port_id].vlan_tci==vlan_tci))||
			(!(pif_dst=find_e3interface_by_index(elan->ports[port_id].iface)))){
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
			{
				uint16_t vlan_tci=LOW_UINT64(fwd_id);
				nr_deliveded=_csp_multicast_forward_slow_path(pif,
					pnode,
					mbufs,
					nr_mbufs,
					vlan_tci);
				drop_start=nr_deliveded;
				nr_dropped=nr_mbufs-drop_start;
			}
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
	int 				idx;
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
	struct csp_cache_entry csp_cache[CSP_CACHE_SIZE];
	struct mac_cache_entry mac_cache[CSP_MAC_CACHE_SIZE];
	struct mac_learning_cache_entry mac_learning_cache[CSP_NODE_BURST_SIZE];
	int nr_mac_learning_cache=0;
	uint64_t ts_now;
	memset(csp_cache,0x0,sizeof(csp_cache));
	memset(mac_cache,0x0,sizeof(mac_cache));
	
	DEF_EXPRESS_DELIVERY_VARS();
	RESET_EXPRESS_DELIVERY_VARS();
	if(PREDICT_FALSE(!pif))
		return 0;
	priv=(struct csp_private*)pif->private;
	nr_rx=rte_eth_rx_burst(iface,queue_id,mbufs,CSP_NODE_BURST_SIZE);
	pre_setup_env(nr_rx);
	while((iptr=peek_next_mbuf())>=0){
		prefetch_next_mbuf(mbufs,iptr);
		fwd_id=_csp_process_input_packet(mbufs[iptr],
			csp_cache,
			mac_cache,
			mac_learning_cache,
			&nr_mac_learning_cache,
			iface,
			priv);
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
	if(nr_mac_learning_cache){
		ts_now=rte_get_tsc_cycles();
		for(idx=0;idx<nr_mac_learning_cache;idx++){
			update_mac_learning_cache(&mac_learning_cache[idx],ts_now);
		}
	}
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
	rte_eth_promiscuous_enable(pif->port_id);
	rte_rwlock_init(&priv->csp_guard);
	/*setup vlan stripping here*/
	return rte_eth_dev_set_vlan_offload(pif->port_id,ETH_VLAN_STRIP_OFFLOAD);
}
int customer_service_port_iface_delete(struct E3Interface * pif)
{
	int idx=0;
	struct csp_private * priv=(struct csp_private*)pif->private;
	/*
	*delete all the ports in e-services,
	*no RWLOCK is aquired,since it's called in RCU context
	*/
	for (idx=0;idx<4096;idx++){
		if(!priv->vlans[idx].is_valid)
			continue;
		switch(priv->vlans[idx].e_service)
		{
			case e_line_service:
				if(delete_e_line_port(priv->vlans[idx].service_index)){
					E3_ERROR("error occurs during detach port %d\n from E-LINE service %d\n",
						pif->port_id,
						priv->vlans[idx].service_index);
				}
				break;
			case e_lan_service:
				{
					int elan_port_id=find_e_lan_port_locked(priv->vlans[idx].service_index,
						pif->port_id,
						idx);
					if(elan_port_id<0)
						break;
					if(delete_e_lan_port(priv->vlans[idx].service_index,elan_port_id)){
						E3_ERROR("error occurs during detach port %d\n from E-LAN service %d\n",
							pif->port_id,
							priv->vlans[idx].service_index);
					}
				}
				break;
			default:
				E3_ERROR("as a matter of fact, it never reach here\n");
				break;
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

void customer_service_port_module_test(void)
{
	struct E3Interface *pif=find_e3interface_by_index(0);
	struct csp_private*priv;
	E3_ASSERT(pif->hwiface_role==E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT);
	E3_ASSERT(priv=(struct csp_private*)pif->private);
	
	struct common_neighbor neighbor={
		.neighbour_ip_as_le=0x12345678,
		.mac={0x08,0x00,0x27,0x53,0x9d,0x44},/*08:00:27:53:9d:44*/
	};
	E3_ASSERT(register_common_neighbor(&neighbor)==0);
	struct common_nexthop nexthop={
		.local_e3iface=0,
		.common_neighbor_index=0,
	};
	E3_ASSERT(register_common_nexthop(&nexthop)==0);

	E3_ASSERT(register_e_line_service()==0);
	E3_ASSERT(!register_e_line_port(0,0,0));
	E3_ASSERT(!register_e_line_nhlfe(0,0,123));
	/*
	*test for e-line service
	*/
	priv->vlans[100].e_service=e_line_service;
	priv->vlans[100].service_index=0;
	priv->vlans[100].is_valid=1;
	
	/*
	test for e-lan multicast forwarding
	*/
	E3_ASSERT(register_e_lan_service()==0);
	E3_ASSERT(register_e_lan_port(0,0,100)>=0);
	E3_ASSERT(register_e_lan_port(0,1,200)>=0);
	struct ether_e_lan *elan=find_e_lan_service(0);
	E3_ASSERT(elan);

	elan->multicast_NHLFE=0;
	elan->multicast_label=1024;
	
	priv->vlans[100].e_service=e_lan_service;
	priv->vlans[100].service_index=0;
	priv->vlans[100].is_valid=1;
	
	//08:00:27:1c:d8:fa
	uint8_t mac[6]={0x08,0x00,0x27,0x1c,0xd8,0xfa};

	/*port entry*/
	struct e_lan_fwd_entry fwd_entry={
		.is_port_entry=1,
		.e3iface=1,
		.vlan_tci=200,
	};
	E3_ASSERT(!register_e_lan_fwd_entry(0,mac,&fwd_entry));
	
	/*MPLS entry*/
	E3_ASSERT(register_e_lan_nhlfe(0,0,1023)>=0);
	struct e_lan_fwd_entry fwd_entry1={
		.is_port_entry=0,
		.NHLFE=0,
		.label_to_push=1023,
	};
	E3_ASSERT(!register_e_lan_fwd_entry(0,mac,&fwd_entry1));
}
