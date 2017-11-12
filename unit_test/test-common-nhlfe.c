/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3test.h>
#include <e3net/include/common-nhlfe.h>
#include <util.h>
DECLARE_TEST_CASE(tc_common_nhlfe);

START_TEST(common_neighbor_general){
	int idx=0;
	int last_offset=0,last_size=0;
	printf("dump definition of common_neighbor:\n");
	dump_field(struct common_neighbor,neighbour_ip_as_le);
	dump_field(struct common_neighbor,ref_cnt);
	dump_field(struct common_neighbor,index);
	dump_field(struct common_neighbor,mac);
	dump_field(struct common_neighbor,is_valid);
	dump_field(struct common_neighbor,reserved0);
	
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		neighbor_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		nexthop_base[idx].is_valid=0;
	
	ck_assert(!!find_common_neighbor(0));
	ck_assert(!!find_common_neighbor(MAX_COMMON_NEIGHBORS-1));
	ck_assert(!find_common_neighbor(MAX_COMMON_NEIGHBORS));
	struct common_neighbor neighbor;
	neighbor.neighbour_ip_as_le=0x1;
	
	/*
	*maximmum number of neighbors supported
	*/
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++){
		neighbor.neighbour_ip_as_le++;
		ck_assert(register_common_neighbor(&neighbor)>=0);
	}
	neighbor.neighbour_ip_as_le++;
	ck_assert(register_common_neighbor(&neighbor)<0);
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++){
		ck_assert(!delete_common_neighbor(idx));
	}
	/*reference count and deletion*/
	ck_assert(reference_common_nrighbor(0)<0);
	ck_assert(reference_common_nrighbor(MAX_COMMON_NEIGHBORS-1)<0);
	ck_assert(reference_common_nrighbor(MAX_COMMON_NEIGHBORS)<0);
	ck_assert(register_common_neighbor(&neighbor)==0);
	ck_assert(!reference_common_nrighbor(0));
	ck_assert(delete_common_neighbor(0));
	ck_assert(!dereference_common_neighbor(0));
	ck_assert(!delete_common_neighbor(0));

	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		neighbor_base[idx].is_valid=0;
}
END_TEST
ADD_TEST(common_neighbor_general);

START_TEST(common_nexthop_general){
	int idx;
	int last_offset=0,last_size=0;
	printf("dump definition of common_nexthop:\n");
	dump_field(struct common_nexthop,local_e3iface);
	dump_field(struct common_nexthop,common_neighbor_index);
	dump_field(struct common_nexthop,ref_cnt);
	dump_field(struct common_nexthop,index);
	dump_field(struct common_nexthop,is_valid);
	dump_field(struct common_nexthop,reserved0);
	dump_field(struct common_nexthop,reserved1);

	ck_assert(!!find_common_nexthop(0));
	ck_assert(!!find_common_nexthop(MAX_COMMON_NEXTHOPS-1));
	ck_assert(!find_common_nexthop(MAX_COMMON_NEXTHOPS));

	/*
	*test registeration
	*/
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		neighbor_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		nexthop_base[idx].is_valid=0;
	struct common_neighbor neighbor;
	struct common_nexthop  nexthop;

	nexthop.local_e3iface=0;
	nexthop.common_neighbor_index=0;
	ck_assert(register_common_nexthop(&nexthop)<0);
	ck_assert(register_common_neighbor(&neighbor)==0);
	ck_assert(register_common_nexthop(&nexthop)==0);
	ck_assert(delete_common_neighbor(0)<0);
	ck_assert(!delete_common_nexthop(0));
	ck_assert(!delete_common_neighbor(0));

	ck_assert(register_common_neighbor(&neighbor)==0);
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++){
		ck_assert(register_common_nexthop(&nexthop)>=0);
		nexthop.local_e3iface++;
	}
	ck_assert(register_common_nexthop(&nexthop)<0);
	ck_assert(delete_common_neighbor(0)<0);
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++){
		ck_assert(!delete_common_nexthop(idx));
	}
	/*
	*check reference count
	*/
	ck_assert(register_common_nexthop(&nexthop)==0);
	ck_assert(reference_common_nexthop(1));
	ck_assert(reference_common_nexthop(MAX_COMMON_NEXTHOPS));
	ck_assert(!reference_common_nexthop(0));
	ck_assert(delete_common_nexthop(0));
	ck_assert(!dereference_common_nexthop(0));
	ck_assert(!delete_common_nexthop(0));
	ck_assert(reference_common_nexthop(0));

	/*clean environment*/
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		neighbor_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		nexthop_base[idx].is_valid=0;
}
END_TEST
ADD_TEST(common_nexthop_general);

