#include <e3iface-wrapper.h>

static int tap_number=0;

static int tap_pre_setup(struct E3Interface * pe3iface)
{
	pe3iface->hwiface_model=e3_hwiface_model_tap;
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

int tap_input_process_func(void* argv)
{
	return 0;
}
int tap_output_process_func(void * argv)
{

	return 0;
}
struct E3Interface_ops tapiface_ops={
	.priv_size=0,
	.numa_socket_id=0,
	.pre_setup=tap_pre_setup,
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
	int rc=register_e3interface(params,ops,&phy_port_id);
	if(rc)
		return rc;
	memset(tap_params,0x0,sizeof(tap_params));
	sprintf(tap_params,"eth_tap%d,iface=e3tap%d",tap_number,tap_number);
	rc=register_e3interface(tap_params,&tapiface_ops,&tap_port_id);
	if(rc){
		unregister_e3interface(phy_port_id);
		return rc;
	}
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
