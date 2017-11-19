#include <e3test.h>
#include <leaf/include/customer-backbone-port.h>
#include <e3infra/include/e3-ini-config.h>
#include <e3infra/include/e3-hashmap.h>
#include <e3net/include/e3iface-inventory.h>
#include <e3net/include/common-cache.h>
#include <e3infra/include/node.h>
#include <e3infra/include/lcore-extension.h>
DECLARE_TEST_CASE(tc_customer_backbone_port);


inline uint64_t _process_cbp_input_packet(struct rte_mbuf* mbuf,
	struct cbp_cache_entry* cbp_cache,
	struct mac_cache_entry* mac_cache,
	struct cbp_private * priv);


START_TEST(cbp_generic){
	/*
	*environmental pre-setup
	*
	*find a port which is E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT
	*if not found, this case will fail
	*/
	
	struct rte_mempool * mempool;
	struct node * pnode;
	int idx=0;
	struct E3Interface * pif=NULL;
	for(idx=0;idx<MAX_NUMBER_OF_E3INTERFACE;idx++){
		pif=find_e3interface_by_index(idx);
		if(pif&&(pif->hwiface_role==E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT))
			break;
	}
	ck_assert_msg(pif&&(pif->hwiface_role==E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT),
		"please provide a  E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT port\n");
	
	pnode=find_node_by_index(pif->input_node[0]);
	mempool=get_mempool_by_socket_id(lcore_to_socket_id(pnode->lcore_id));
	ck_assert(!!mempool);
	/*
	*environmental cleanup
	*/
}
END_TEST
ADD_TEST(cbp_generic);
