#include <leaf/include/customer-service-port.h>
#include <e3_init.h>
#include <e3iface-inventory.h>
#include <leaf/include/leaf-e-service.h>
#include <leaf/include/leaf-label-fib.h>
extern struct e3iface_role_def  role_defs[E3IFACE_ROLE_MAX_ROLES];
static int csp_capability_check(int port_id)
{
	struct rte_eth_dev_info dev_info;
	rte_eth_dev_info_get(port_id,&dev_info);
	if(!(dev_info.rx_offload_capa&DEV_RX_OFFLOAD_VLAN_STRIP)||
		!(dev_info.tx_offload_capa&DEV_TX_OFFLOAD_VLAN_INSERT))
		return -1;
	return 0;
}

int customer_service_port_iface_input_iface(void * arg)
{
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

