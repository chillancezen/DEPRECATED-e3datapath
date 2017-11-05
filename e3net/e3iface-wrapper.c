#include <e3iface-wrapper.h>
#include <e3iface-inventory.h>
#include <node.h>
#include <mbuf_delivery.h>
static int tap_number=0;

static int tap_pre_setup(struct E3Interface * pe3iface)
{
	pe3iface->hwiface_model=E3IFACE_MODEL_TAP_SINGLY_QUEUE;
	pe3iface->hwiface_role=E3IFACE_ROLE_HOST_STACK_PORT;
	pe3iface->nr_queues=1;
	return 0;
}
static int tap_port_config(struct E3Interface * piface,struct rte_eth_conf * port_config)
{
	port_config->rxmode.mq_mode=ETH_MQ_RX_NONE;
	port_config->rxmode.max_rx_pkt_len=ETHER_MAX_LEN;
	port_config->rxmode.header_split=0;
	port_config->rxmode.hw_ip_checksum=1;
	port_config->rxmode.hw_vlan_filter=0;
	port_config->rxmode.hw_vlan_strip=0;
	port_config->rxmode.hw_vlan_extend=0;
	port_config->rxmode.jumbo_frame=0;
	port_config->rxmode.hw_strip_crc=0;
	port_config->rxmode.enable_scatter=0;
	port_config->rxmode.enable_lro=0;
	port_config->txmode.mq_mode=ETH_MQ_TX_NONE;
	return 0;
}
/*
*read pkts from tap device, and send to corresponding 
*peer device immediately
*/
int tap_input_process_func(void* argv)
{
	#define _(c) if(!(c)) goto ret
	struct rte_mbuf *mbufs[32];
	int nr_rx;
	int nr_tx;
	int idx=0;
	struct node * pnode=(struct node *)argv;
	int iface_id=HIGH_UINT64((uint64_t)pnode->node_priv);
	int queue_id=LOW_UINT64((uint64_t)pnode->node_priv);
	struct E3Interface * pif=find_e3interface_by_index(iface_id);
	struct E3Interface * peer_if;
	_(pif&&pif->has_peer_device);
	peer_if=find_e3interface_by_index(pif->peer_port_id);
	_(peer_if);
	nr_rx=rte_eth_rx_burst(iface_id,queue_id,mbufs,32);
	_(nr_rx);
	nr_tx=deliver_mbufs_to_e3iface(peer_if,0,mbufs,nr_rx);
	for(idx=nr_tx;idx<nr_rx;idx++)
		rte_pktmbuf_free(mbufs[idx]);
	ret:
		return 0;
	#undef _
}
int tap_output_process_func(void * argv)
{
	#define _(c) if(!(c)) goto ret
	int idx=0;
	struct rte_mbuf *mbufs[32];
	int nr_rx;
	int nr_tx;
	struct node * pnode=(struct node *)argv;
	int iface_id=HIGH_UINT64((uint64_t)pnode->node_priv);
	int queue_id=LOW_UINT64((uint64_t)pnode->node_priv);
	struct E3Interface * pif=find_e3interface_by_index(iface_id);
	_(pif);
	nr_rx=rte_ring_sc_dequeue_burst(pnode->node_ring,(void**)mbufs,32,NULL);
	_(nr_rx);
	nr_tx=rte_eth_tx_burst(iface_id,queue_id,mbufs,nr_rx);
	for(idx=nr_tx;idx<nr_rx;idx++)
		rte_pktmbuf_free(mbufs[idx]);
	ret:
		return 0;
}
struct E3Interface_ops tapiface_ops={
	.priv_size=0,
	.numa_socket_id=0,
	.queue_setup=tap_pre_setup,
	.port_setup=tap_port_config,
	.input_node_process_func=tap_input_process_func,
	.output_node_process_func=tap_output_process_func,
	.edges={
		{.edge_entry=-1,},
	},
};
int create_e3iface_with_slowpath(char * params,struct E3Interface_ops * ops,int *pport_id)
{
	char tap_params[64];
	int phy_port_id=0;
	int tap_port_id=0;
	struct E3Interface * pe3if_phy=NULL;
	struct E3Interface * pe3if_tap=NULL;
	int rc=register_e3interface(params,ops,&phy_port_id);
	if(rc)
		return rc;
	E3_ASSERT(pe3if_phy=find_e3interface_by_index(phy_port_id));
	memset(tap_params,0x0,sizeof(tap_params));
	sprintf(tap_params,"eth_tap%d,iface=e3tap%d,speed=10000"
		,tap_number
		,tap_number);
	rc=register_e3interface(tap_params,&tapiface_ops,&tap_port_id);
	if(rc){
		unregister_e3interface(phy_port_id);
		return rc;
	}
	/*after creating the tap device, set its mac same with physical one*/
	pe3if_tap=find_e3interface_by_index(tap_port_id);
	E3_ASSERT(pe3if_tap);
	rte_eth_dev_default_mac_addr_set(tap_port_id,(struct ether_addr *)pe3if_phy->mac_addrs);
	rte_memcpy(pe3if_tap->mac_addrs,pe3if_phy->mac_addrs,6);
	
	E3_ASSERT(!correlate_e3interfaces(find_e3interface_by_index(phy_port_id),find_e3interface_by_index(tap_port_id)));
	tap_number++;
	if(pport_id)
		*pport_id=phy_port_id;
	return 0;
}
int release_e3iface_with_slowpath(int any_port_id)
{

	int peer_port_id=-1;
	struct E3Interface * pif=find_e3interface_by_index(any_port_id);
	if(!pif)
		return -1;
	if(pif->has_peer_device){
		peer_port_id=pif->peer_port_id;
		E3_ASSERT(!dissociate_e3interface(pif));
	}
	unregister_e3interface(any_port_id);
	if(peer_port_id!=-1)
		unregister_e3interface(peer_port_id);
	return 0;
}



/*helper functions to be exported*/

