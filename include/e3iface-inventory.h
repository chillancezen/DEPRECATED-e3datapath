#ifndef _E3IFACE_INVENTORY_H
#define _E3IFACE_INVENTORY_H
#include <e3iface-wrapper.h>

/*split dev_ops into 2 Dimensions as below
*since this is the inventory,
we enumerate all supported device model here*/

enum e3iface_model{
	E3IFACE_MODEL_GENERIC_SINGLY_QUEUE=0,/*compatiable with lots of dpdk compatible (v)NICs*/
	E3IFACE_MODEL_TAP_SINGLY_QUEUE, /*do not export this model,but it's still used to distinguish*/
	E3IFACE_MODEL_INTEL_XL710_SINGLY_QUEUE,
	E3IFACE_MODEL_INTEL_XL710_VF_SINGLY_QUEUE,
	E3IFACE_MODEL_MAX_MODELS,
};
enum e3iface_role{
	E3IFACE_ROLE_PROVIDER_BACKBONE_PORT=0,/*Provider backbone port*/
	E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,/*customer backbone port*/
	E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,/*customer service port*/
	E3IFACE_ROLE_HOST_STACK_PORT,
	E3IFACE_ROLE_MAX_ROLES,
};
struct e3iface_model_def{
	int is_set;
	int check_lsc;
	
	int (*queue_setup)(struct E3Interface * iface);
	int (*port_setup)(struct E3Interface * iface,struct rte_eth_conf * port_conf);
	int (*lsc_iface_up)(int iface);
	int (*lsc_iface_down)(int iface);
};

struct e3iface_role_def{
	int is_set;
	int priv_size;
	int (*capability_check)(int port_id);
	int (*input_node_process_func)(void * arg);
	int (*output_node_process_func)(void * arg);
	int (*post_setup)(struct E3Interface * iface);
	int (*iface_delete)(struct E3Interface*);
	struct next_edge_item edges[MAX_PREDEFINED_EDGE];
};
#endif