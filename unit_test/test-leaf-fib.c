/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3test.h>
#include <leaf/include/leaf-label-fib.h>
#include <leaf/include/leaf-e-service.h>
#include <rte_malloc.h>
DECLARE_TEST_CASE(tc_leaf_fib);

START_TEST(leaf_fib_general){
	int idx=0;
	struct leaf_label_entry * base=allocate_leaf_label_base(-1);
	struct leaf_label_entry * base1=allocate_leaf_label_base(-1);
	ck_assert_msg(!!base,"OUT OF MEMORY,please reserve enough memory for test leaf fib");
	ck_assert_msg(!!base1,"OUT OF MEMORY,please reserve enough memory for test leaf fib");
	ck_assert(!!leaf_label_entry_at(base,0));
	ck_assert(!!leaf_label_entry_at(base,NR_LEAF_LABEL_ENTRY-1));
	ck_assert(!leaf_label_entry_at(base,NR_LEAF_LABEL_ENTRY));
	/*environmrntal presetup*/
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		e_line_base[idx].is_valid=0;
	for(idx=0;idx<MAX_E_LAN_SERVICES;idx++){
		e_lan_base[idx].is_valid=0;
		e_lan_base[idx].is_releasing=0;
	}
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		neighbor_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		nexthop_base[idx].is_valid=0;
	struct common_neighbor neighbor;
	struct common_nexthop  nexthop={
		.common_neighbor_index=0,
	};
	ck_assert(register_common_neighbor(&neighbor)==0);
	ck_assert(register_common_nexthop(&nexthop)==0);
	
	/*
	*e-line-service fib test
	*/
	struct ether_e_line eline;
	eline.e3iface=0;
	eline.vlan_tci=12;
	eline.label_to_push=0x322;
	eline.NHLFE=0;
	ck_assert(register_e_line_service()==0);
	ck_assert(!register_e_line_port(0,eline.e3iface,eline.vlan_tci));
	ck_assert(!register_e_line_nhlfe(0,eline.NHLFE,eline.label_to_push));
	
	eline.vlan_tci++;
	eline.label_to_push++;
	ck_assert(register_e_line_service()==1);
	ck_assert(!register_e_line_port(1,eline.e3iface,eline.vlan_tci));
	ck_assert(!register_e_line_nhlfe(1,eline.NHLFE,eline.label_to_push));
	/*
	*due to register_e_line_nhlfe() api modification,
	* change test cases as well
	*/
	ck_assert(find_e_line_service(0)->ref_cnt==1);
	ck_assert(find_e_line_service(1)->ref_cnt==1);
	
	struct leaf_label_entry entry;
	entry.e3_service=e_line_service+3;
	entry.service_index=0;

	ck_assert(set_leaf_label_entry(base,0,&entry)<0);
	entry.e3_service=e_line_service;
	entry.service_index=2;
	ck_assert(set_leaf_label_entry(base,0,&entry)<0);
	entry.e3_service=e_line_service;
	entry.service_index=0;
	ck_assert(!set_leaf_label_entry(base,0,&entry));
	ck_assert(find_e_line_service(0)->ref_cnt==2);
	ck_assert(find_e_line_service(1)->ref_cnt==1);
	ck_assert(!set_leaf_label_entry(base,NR_LEAF_LABEL_ENTRY-1,&entry));
	ck_assert(set_leaf_label_entry(base,NR_LEAF_LABEL_ENTRY,&entry)<0);
	ck_assert(find_e_line_service(0)->ref_cnt==2);
	ck_assert(find_e_line_service(1)->ref_cnt==1);
	entry.e3_service=e_line_service;
	entry.service_index=1;
	ck_assert(!set_leaf_label_entry(base,NR_LEAF_LABEL_ENTRY-1,&entry));
	ck_assert(find_e_line_service(0)->ref_cnt==2);
	ck_assert(find_e_line_service(1)->ref_cnt==2);
	entry.e3_service=e_line_service;
	entry.service_index=1;
	ck_assert(!set_leaf_label_entry(base,0,&entry));
	ck_assert(find_e_line_service(0)->ref_cnt==1);
	ck_assert(find_e_line_service(1)->ref_cnt==2);
	reset_leaf_label_entry(base,0);
	ck_assert(find_e_line_service(0)->ref_cnt==1);
	ck_assert(find_e_line_service(1)->ref_cnt==2);
	reset_leaf_label_entry(base,NR_LEAF_LABEL_ENTRY-1);
	ck_assert(find_e_line_service(0)->ref_cnt==1);
	ck_assert(find_e_line_service(1)->ref_cnt==1);

	entry.e3_service=e_line_service;
	entry.service_index=0;

	ck_assert(!set_leaf_label_entry(base,0,&entry));
	ck_assert(!set_leaf_label_entry(base1,NR_LEAF_LABEL_ENTRY-1,&entry));
	ck_assert(find_e_line_service(0)->ref_cnt==3);
	reset_leaf_label_entry(base,0);
	ck_assert(find_e_line_service(0)->ref_cnt==2);
	reset_leaf_label_entry(base1,NR_LEAF_LABEL_ENTRY-1);
	ck_assert(find_e_line_service(0)->ref_cnt==1);
	/*
	*e-lan-service test
	*/
	ck_assert(register_e_lan_service()==0);
	ck_assert(register_e_lan_service()==1);

	entry.e3_service=e_lan_service;
	entry.service_index=0;
	ck_assert(set_leaf_label_entry(base,NR_LEAF_LABEL_ENTRY,&entry));
	entry.service_index=2;
	ck_assert(set_leaf_label_entry(base,0,&entry));
	entry.service_index=0;
	ck_assert(!set_leaf_label_entry(base,0,&entry));
	ck_assert(find_e_lan_service(0)->ref_cnt==1);
	ck_assert(!set_leaf_label_entry(base,NR_LEAF_LABEL_ENTRY-1,&entry));
	ck_assert(find_e_lan_service(0)->ref_cnt==1);
	
	entry.service_index=1;
	ck_assert(!set_leaf_label_entry(base,0,&entry));
	ck_assert(find_e_lan_service(0)->ref_cnt==1);
	ck_assert(find_e_lan_service(1)->ref_cnt==1);

	reset_leaf_label_entry(base,NR_LEAF_LABEL_ENTRY-1);
	ck_assert(find_e_lan_service(0)->ref_cnt==0);
	ck_assert(find_e_lan_service(1)->ref_cnt==1);
	reset_leaf_label_entry(base,0);
	ck_assert(find_e_lan_service(0)->ref_cnt==0);
	ck_assert(find_e_lan_service(1)->ref_cnt==0);

	entry.e3_service=e_lan_service;
	entry.service_index=0;

	ck_assert(!set_leaf_label_entry(base,0,&entry));
	ck_assert(!set_leaf_label_entry(base1,NR_LEAF_LABEL_ENTRY-1,&entry));
	ck_assert(find_e_lan_service(0)->ref_cnt==2);
	reset_leaf_label_entry(base,0);
	ck_assert(find_e_lan_service(0)->ref_cnt==1);
	reset_leaf_label_entry(base1,NR_LEAF_LABEL_ENTRY-1);
	ck_assert(find_e_lan_service(0)->ref_cnt==0);


	entry.e3_service=e_lan_service;
	entry.service_index=0;
	ck_assert(!set_leaf_label_entry(base,507,&entry));
	ck_assert(set_leaf_label_entry_egress_nhlfe_index(base,508,0,100));
	ck_assert(set_leaf_label_entry_egress_nhlfe_index(base,507,0,100));
	ck_assert(register_e_lan_nhlfe(0,0,100)>=0);
	ck_assert(!set_leaf_label_entry_egress_nhlfe_index(base,507,0,100));
	ck_assert(set_leaf_label_entry_egress_nhlfe_index(base,508,0,100));
	ck_assert(clear_leaf_label_entry_egress_nhlfe_index(base,508));
	ck_assert(!clear_leaf_label_entry_egress_nhlfe_index(base,507));
	/*environmrntal post-setup*/
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		e_line_base[idx].is_valid=0;
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		e_lan_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		neighbor_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		nexthop_base[idx].is_valid=0;
	RTE_FREE(base);
	RTE_FREE(base1);
}
END_TEST
ADD_TEST(leaf_fib_general);
