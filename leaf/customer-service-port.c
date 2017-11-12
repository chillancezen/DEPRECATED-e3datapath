/*
*Copyright (c) 2016-2017 Jie Zheng
*/
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
extern struct e3iface_role_def  role_defs[E3IFACE_ROLE_MAX_ROLES];
#define CSP_NODE_BURST_SIZE 48


#define CSP_PROCESS_INPUT_DROP 0x0
#define CSP_PROCESS_INPUT_ELINE_FWD 0x1
#define CSP_PROCESS_INPUT_ELAN_UNICAST_FWD 0x2
#define CSP_PROCESS_INPUT_ELAN_MULTICAST_FWD 0x3
#define CSP_PROCESS_INPUT_HOST_STACK 0x4

struct csp_cache_entry{
	uint16_t is_valid;
	uint16_t vlan_tci;
	uint32_t is_e_line;
	struct ether_e_lan * elan;
	struct ether_e_line * eline;

	/*e_line specific fields*/
	struct common_nexthop *	eline_nexthop;
	struct E3Interface	  *	eline_src_pif;
	struct common_neighbor*	eline_neighbor;
};

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
	struct csp_private * priv)
{
	uint64_t fwd_id=MAKE_UINT64(CSP_PROCESS_INPUT_DROP,0);	
	struct ether_hdr *eth_hdr=rte_pktmbuf_mtod(mbuf,struct ether_hdr*);
	uint16_t pkt_vlan_tci=0;
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
		
	}
	ret:
	return fwd_id;
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
		fwd_id=_csp_process_input_packet(mbufs[iptr],&csp_cache,priv);
		process_rc=proceed_mbuf(iptr,fwd_id);
		if(process_rc==MBUF_PROCESS_RESTART){
			fetch_pending_index(start_index,end_index);
			fetch_pending_fwd_id(last_fwd_id);
			
			flush_pending_mbuf();
			proceed_mbuf(iptr,fwd_id);
		}
	}
	fetch_pending_index(start_index,end_index);
	fetch_pending_fwd_id(last_fwd_id);
	
	return 0;
}
int customer_service_port_iface_output_iface(void * arg)
{

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

