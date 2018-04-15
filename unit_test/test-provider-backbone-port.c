/*
*Copyright (c) 2016-2017 Jie Zheng
*/

#include <e3test.h>
#include <spine/include/provider-backbone-port.h>
#include <e3net/include/e3iface-inventory.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <e3infra/include/util.h>
#include <e3net/include/mpls-util.h>
#include <e3infra/include/node.h>
#include <rte_mempool.h>
#include <e3infra/include/lcore-extension.h>
#include <e3net/include/common-cache.h>
#include <e3api/include/e3-api-wrapper.h>
#include <rte_ether.h>


inline uint64_t _process_pbp_input_packet(struct rte_mbuf * mbuf,
											struct pbp_cache_entry * pbp_cache,
											struct pbp_private     * priv);

e3_type spine_api_pbp_setup_label_entry(e3_type service,
							e3_type iface,/*input*/
							e3_type index_to_set,/*input*/
							e3_type entry/*input*/);

e3_type spine_api_pbp_delete_label_entry(e3_type service,
		e3_type e3iface,
		e3_type label_index);



DECLARE_TEST_CASE(tc_provider_backbone_port);


START_TEST(pbp_generic){
	/*
	*find a provider backbone port
	*/
	int idx;
	uint64_t fwd_id;
	struct E3Interface * pif=NULL;
	struct pbp_private* priv;
	struct node * pnode;
	struct rte_mempool * mempool;
	struct rte_mbuf * mbuf;
	struct ether_hdr * eth_hdr;
	struct mpls_hdr * mpls;
	int label;
	for(idx=0;idx<MAX_NUMBER_OF_E3INTERFACE;idx++){
		pif=find_e3interface_by_index(idx);
		if(pif&&(pif->hwiface_role==E3IFACE_ROLE_PROVIDER_BACKBONE_PORT))
			break;
	}
	ck_assert_msg(!!pif,"please provide a e3interface with role of E3IFACE_ROLE_PROVIDER_BACKBONE_PORT");
	struct pbp_cache_entry pbp_cache[PBP_CACHE_SIZE];
	priv=(struct pbp_private*)pif->private;
	pnode=find_node_by_index(pif->input_node[0]);
	ck_assert(!!pnode);
	mempool=get_mempool_by_socket_id(lcore_to_socket_id(pnode->lcore_id));
	ck_assert(!!mempool);
	
	/*
	*register a neighbor&nexthop
	*/
	struct common_neighbor neighbor={
        .name = "hello world",
		.mac={0x12,0x12,0x12,0x12,0x12,0x12},
	};
	ck_assert(register_common_neighbor(&neighbor)==0);
	
	struct common_nexthop nexthop={
		.local_e3iface=0,
		.common_neighbor_index=0,
	};
	ck_assert(register_common_nexthop(&nexthop)==0);

	/*
	*register a spine label unicast entry
	*/
	struct spine_label_entry lentry;
	lentry.is_unicast=1;
	lentry.NHLFE=0;
	lentry.swapped_label=0x23453;
	ck_assert(!spine_api_pbp_setup_label_entry(0,
		pif->port_id,
		34567,
		(e3_type)&lentry));

	/*
	*forwarding with invalid label
	*/
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,64)));
	memset(eth_hdr,0x0,64);
	eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
	mpls=(struct mpls_hdr*)(eth_hdr+1);
	label=34568;
	set_mpls_label(mpls,label);
	set_mpls_exp(mpls,0);
	set_mpls_bottom(mpls);
	set_mpls_ttl(mpls,64);
	fwd_id=_process_pbp_input_packet(mbuf,
		pbp_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==PBP_PROCESS_INPUT_DROP);
	rte_pktmbuf_free(mbuf);
	memset(pbp_cache,0x0,sizeof(pbp_cache));
	
	/*
	*forwarding with valid unicast label
	*/
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,64)));
	memset(eth_hdr,0x0,64);
	eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
	mpls=(struct mpls_hdr*)(eth_hdr+1);
	label=34567;
	set_mpls_label(mpls,label);
	set_mpls_exp(mpls,0);
	set_mpls_bottom(mpls);
	set_mpls_ttl(mpls,64);
	fwd_id=_process_pbp_input_packet(mbuf,
		pbp_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==PBP_PROCESS_INPUT_UNICAST_FWD);
	ck_assert(eth_hdr->ether_type==ETHER_PROTO_MPLS_UNICAST);
	ck_assert(mpls_label(mpls)==lentry.swapped_label);
	rte_pktmbuf_free(mbuf);
	memset(pbp_cache,0x0,sizeof(pbp_cache));
	/*
	*forwarding with valid unicast label with invalid TTL value
	*/
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,64)));
	memset(eth_hdr,0x0,64);
	eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
	mpls=(struct mpls_hdr*)(eth_hdr+1);
	label=34567;
	set_mpls_label(mpls,label);
	set_mpls_exp(mpls,0);
	set_mpls_bottom(mpls);
	set_mpls_ttl(mpls,0);
	fwd_id=_process_pbp_input_packet(mbuf,
		pbp_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==PBP_PROCESS_INPUT_DROP);
	rte_pktmbuf_free(mbuf);
	memset(pbp_cache,0x0,sizeof(pbp_cache));
	/*
	*forwarding with valid multicast label entry
	*/
	ck_assert(register_multicast_nexthop()==0);
	
	lentry.is_unicast=0;
	lentry.NHLFE=0;
	lentry.swapped_label=0x23453;
	ck_assert(!spine_api_pbp_setup_label_entry(0,
		pif->port_id,
		34567,
		(e3_type)&lentry));
	
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,64)));
	memset(eth_hdr,0x0,64);
	eth_hdr->ether_type=ETHER_PROTO_MPLS_UNICAST;
	mpls=(struct mpls_hdr*)(eth_hdr+1);
	label=34567;
	set_mpls_label(mpls,label);
	set_mpls_exp(mpls,0);
	set_mpls_bottom(mpls);
	set_mpls_ttl(mpls,64);
	fwd_id=_process_pbp_input_packet(mbuf,
		pbp_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==PBP_PROCESS_INPUT_MULTICAST_FWD);
	rte_pktmbuf_free(mbuf);
	memset(pbp_cache,0x0,sizeof(pbp_cache));

	/*
	*host stack forwarding
	*/
	lentry.is_unicast=0;
	lentry.NHLFE=0;
	lentry.swapped_label=0x23453;
	ck_assert(!spine_api_pbp_setup_label_entry(0,
		pif->port_id,
		34567,
		(e3_type)&lentry));
	
	mbuf=rte_pktmbuf_alloc(mempool);
	ck_assert(!!mbuf);
	ck_assert(!!(eth_hdr=(struct ether_hdr*)rte_pktmbuf_append(mbuf,64)));
	memset(eth_hdr,0x0,64);
	eth_hdr->ether_type=0x0008;
	fwd_id=_process_pbp_input_packet(mbuf,
		pbp_cache,
		priv);
	ck_assert(HIGH_UINT64(fwd_id)==PBP_PROCESS_INPUT_HOST_STACK);
	rte_pktmbuf_free(mbuf);
	memset(pbp_cache,0x0,sizeof(pbp_cache));

	/*
	*resource reclaim
	*/
	ck_assert(!spine_api_pbp_delete_label_entry(0,pif->port_id,34567));
	ck_assert(!delete_multicast_nexthop(0));
	find_common_nexthop(0)->ref_cnt=0;
	find_common_neighbor(0)->ref_cnt=0;
	ck_assert(!delete_common_nexthop(0));
	ck_assert(!delete_common_neighbor(0));
}
END_TEST
ADD_TEST(pbp_generic);

