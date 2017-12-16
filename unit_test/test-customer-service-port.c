/*
*Copyright (c) 2016-2017 Jie Zheng
*/

#include <e3test.h>
#include <leaf/include/customer-service-port.h>
#include <e3net/include/e3iface-inventory.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <e3infra/include/util.h>
#include <e3net/include/mpls-util.h>
#include <e3infra/include/node.h>
#include <rte_mempool.h>
#include <e3infra/include/lcore-extension.h>
#include <e3net/include/common-cache.h>
#include <leaf/include/mac-learning.h>
#include <e3api/include/e3-api-wrapper.h>

DECLARE_TEST_CASE(tc_customer_service_port);

inline uint64_t _csp_process_input_packet(struct rte_mbuf*mbuf,
	struct csp_cache_entry *_csp_cache,
	struct mac_cache_entry *_mac_cache,
	struct mac_learning_cache_entry *mac_learning_cache,
	int	*  nr_mac_learning_cache,
	int e3iface,
	struct csp_private * priv);

e3_type leaf_api_csp_setup_port(e3_type e3service,
	e3_type iface_id,
	e3_type vlan_tci,
	e3_type is_eline_service,
	e3_type service_id);

START_TEST(csp_generic){
	/*
	*first is to find a customer service port
	*/
	
	int idx=0;
	struct E3Interface * pif;
	struct csp_private * priv;
	struct node * pnode;
	struct rte_mempool * mempool;
	struct csp_cache_entry csp_cache[CSP_CACHE_SIZE];
	struct mac_cache_entry mac_cache[CSP_MAC_CACHE_SIZE];
	struct mac_learning_cache_entry mac_learning_cache[CSP_NODE_BURST_SIZE];
	
	for(idx=0;idx<MAX_NUMBER_OF_E3INTERFACE;idx++){
		pif=find_e3interface_by_index(idx);
		if(pif&&(pif->hwiface_role==E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT))
			break;
	}
	ck_assert_msg(!!pif,"please provide a customer service port with E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT role");
	priv=(struct csp_private*)pif->private;
	pnode=find_node_by_index(pif->input_node[0]);
	ck_assert(!!pnode);
	mempool=get_mempool_by_socket_id(lcore_to_socket_id(pnode->lcore_id));
	ck_assert(!!mempool);

	memset(csp_cache,0x0,sizeof(csp_cache));
	memset(mac_cache,0x0,sizeof(mac_cache));
	memset(mac_learning_cache,0x0,sizeof(mac_learning_cache));

	/*
	*neighbours & nexthops & e-line service setup
	*/
	struct common_neighbor neighbor={
		.neighbour_ip_as_le=0x12345678,
		.mac={0x12,0x12,0x12,0x12,0x12,0x12},
	};
	ck_assert(register_common_neighbor(&neighbor)==0);
	
	struct common_nexthop nexthop={
		.local_e3iface=0,
		.common_neighbor_index=0,
	};
	ck_assert(register_common_nexthop(&nexthop)==0);
	
	struct ether_e_line eline={
		.e3iface=0,
		.vlan_tci=123,
		.label_to_push=0x3455,
		.NHLFE=0,
		
	};
	ck_assert(register_e_line_service()==0);
	ck_assert(!register_e_line_nhlfe(0,eline.NHLFE,eline.label_to_push));
	ck_assert(find_e_line_service(0)->is_cbp_ready);
	ck_assert(!find_e_line_service(0)->is_csp_ready);
	/*
	*port vlan distribution entry setup
	*/
	ck_assert(!leaf_api_csp_setup_port(0,
		pif->port_id,
		100,
		1,
		0));
	/*
	*Ether-LINE forwarding test
	*/

	
	
}
END_TEST
ADD_TEST(csp_generic);

