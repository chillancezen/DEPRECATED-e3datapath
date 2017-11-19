#include <e3test.h>
#include <leaf/include/customer-backbone-port.h>
#include <e3infra/include/e3-ini-config.h>
#include <e3infra/include/e3-hashmap.h>
#include <e3net/include/e3iface-inventory.h>
#include <e3net/include/common-cache.h>
#include <e3infra/include/node.h>
#include <e3infra/include/lcore-extension.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <e3infra/include/util.h>
#include <e3net/include/mpls-util.h>

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
	struct ether_hdr * eth_hdr,*inner_eth_hdr;
	struct mpls_hdr  * mpls_hdr;
	uint64_t fwd_id;
	uint32_t label=0;
	uint32_t label_index=0;
	int mac_index;
	struct rte_mempool * mempool;
	struct rte_mbuf * mbuf;
	struct cbp_private * priv;
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
	priv=(struct cbp_private*)pif->private;
	pnode=find_node_by_index(pif->input_node[0]);
	mempool=get_mempool_by_socket_id(lcore_to_socket_id(pnode->lcore_id));
	ck_assert(!!mempool);
	
	struct cbp_cache_entry cbp_cache[CBP_CACHE_SIZE];
	struct mac_cache_entry mac_cache[MAC_CACHE_SIZE];
	memset(cbp_cache,0x0,sizeof(cbp_cache));
	memset(mac_cache,0x0,sizeof(mac_cache));

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
	ck_assert(register_e_line_service(&eline)==0);

	struct leaf_label_entry lentry0={
		.e3_service=e_line_service,
		.service_index=0,
		
	};
	ck_assert(!set_leaf_label_entry(priv->label_base,0x1234,&lentry0));

	ck_assert(register_e_lan_service()==0);
	ck_assert(register_e_lan_port(0,0,234)==0);
	ck_assert(register_e_lan_port(0,1,234)==1);
	
	struct leaf_label_entry lentry={
		.e3_service=e_lan_service,
		.service_index=0,
	};
	ck_assert(!set_leaf_label_entry(priv->label_base,0x2345,&lentry));
	ck_assert(!set_leaf_label_entry(priv->label_base,0x2346,&lentry));
	
	struct e_lan_fwd_entry fwd_entry={
		.is_port_entry=1,
		.e3iface=0,
		.vlan_tci=234,
	};
	//08:00:27:53:9d:44
	uint8_t dst_mac[6]={0x08,0x00,0x27,0x53,0x9d,0x44};
	ck_assert(!register_e_lan_fwd_entry(0,dst_mac,&fwd_entry));
	
	/*host fwd verification*/
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,82)));
	memset(eth_hdr,0x0,82);
	eth_hdr->ether_type=0x0008;
	fwd_id=_process_cbp_input_packet(mbuf,
		cbp_cache,
		mac_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==CBP_PROCESS_INPUT_HOST_STACK);

	/*verification of mpls packet in general*/
	eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
	mpls_hdr=(struct mpls_hdr*)(eth_hdr+1);
	fwd_id=_process_cbp_input_packet(mbuf,
		cbp_cache,
		mac_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==CBP_PROCESS_INPUT_DROP);
	label=0x1235;
	set_mpls_ttl(mpls_hdr,60);
	set_mpls_label(mpls_hdr,label);
	fwd_id=_process_cbp_input_packet(mbuf,
		cbp_cache,
		mac_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==CBP_PROCESS_INPUT_DROP);

	/*e line unicast forwarding*/
	label=0x1234;
	set_mpls_label(mpls_hdr,label);
	inner_eth_hdr=(struct ether_hdr*)(mpls_hdr+1);
	
	fwd_id=_process_cbp_input_packet(mbuf,
		cbp_cache,
		mac_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==CBP_PROCESS_INPUT_ELINE_FWD);
	ck_assert(mbuf->pkt_len==64);
	ck_assert(!!(mbuf->ol_flags&PKT_TX_VLAN_PKT));
	ck_assert(mbuf->vlan_tci=123);

	label_index=label&CBP_CACHE_MASK;
	ck_assert(cbp_cache[label_index].label==label);
	ck_assert(cbp_cache[label_index].is_valid);
	rte_pktmbuf_free(mbuf);
	
	/*e LAN unicast forwarding*/
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,82)));
	memset(eth_hdr,0x0,82);
	eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
	mpls_hdr=(struct mpls_hdr*)(eth_hdr+1);
	inner_eth_hdr=(struct ether_hdr*)(mpls_hdr+1);
	
	memset(cbp_cache,0x0,sizeof(cbp_cache));
	memset(mac_cache,0x0,sizeof(mac_cache));
	rte_memcpy(inner_eth_hdr->d_addr.addr_bytes,
		dst_mac,
		6);
	//inner_eth_hdr->d_addr.addr_byes[5]=0x45;
	label=0x2345;
	set_mpls_label(mpls_hdr,label);
	set_mpls_ttl(mpls_hdr,64);
	fwd_id=_process_cbp_input_packet(mbuf,
		cbp_cache,
		mac_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==CBP_PROCESS_INPUT_ELAN_UNICAST_FWD);
	label_index=label&CBP_CACHE_MASK;
	ck_assert(cbp_cache[label_index].label==label);
	ck_assert(cbp_cache[label_index].is_valid);
	ck_assert(mbuf->pkt_len==64);
	ck_assert(!!(mbuf->ol_flags&PKT_TX_VLAN_PKT));
	ck_assert(mbuf->vlan_tci=234);
	mac_index=dst_mac[5]&MAC_CACHE_MASK;
	ck_assert(IS_MAC_EQUAL(mac_cache[mac_index].mac,dst_mac));
	ck_assert(mac_cache[mac_index].is_valid);
	rte_pktmbuf_free(mbuf);

	/*e LAN multicast forwarding*/
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,82)));
	memset(eth_hdr,0x0,82);
	eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
	mpls_hdr=(struct mpls_hdr*)(eth_hdr+1);
	inner_eth_hdr=(struct ether_hdr*)(mpls_hdr+1);
	
	memset(cbp_cache,0x0,sizeof(cbp_cache));
	memset(mac_cache,0x0,sizeof(mac_cache));
	dst_mac[5]=0x45;
	rte_memcpy(inner_eth_hdr->d_addr.addr_bytes,
		dst_mac,
		6);
	label=0x2345;
	set_mpls_label(mpls_hdr,label);
	set_mpls_ttl(mpls_hdr,64);
	fwd_id=_process_cbp_input_packet(mbuf,
		cbp_cache,
		mac_cache,
		priv);
	/*
	*can not find such mac entry any more
	*due to lacking entry for such dst entry
	*/
	ck_assert(HIGH_UINT64(fwd_id)==CBP_PROCESS_INPUT_ELAN_MULTICAST_FWD);
	label_index=label&CBP_CACHE_MASK;
	ck_assert(cbp_cache[label_index].label==label);
	ck_assert(cbp_cache[label_index].is_valid);
	ck_assert(mbuf->pkt_len==64);
	mac_index=dst_mac[5]&MAC_CACHE_MASK;
	ck_assert(IS_MAC_EQUAL(mac_cache[mac_index].mac,dst_mac));
	ck_assert(!mac_cache[mac_index].is_valid);
	rte_pktmbuf_free(mbuf);
	
	/*
	*environmental cleanup
	*/
	delete_e_lan_port(0,0);
	delete_e_lan_port(0,1);
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		e_line_base[idx].is_valid=0;
	for(idx=0;idx<MAX_E_LAN_SERVICES;idx++)
		e_lan_base[idx].is_valid=0;
}
END_TEST
ADD_TEST(cbp_generic);
