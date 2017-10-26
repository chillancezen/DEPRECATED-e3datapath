#include <e3test.h>
#include <leaf/include/leaf-e-service.h>
#include <util.h>

DECLARE_TEST_CASE(tc_leaf_e_service);

START_TEST(leaf_e_service_general){
	int idx=0;
	int last_offset=0,last_size=0;
	printf("dump definition of ether_e_line:\n");
	dump_field(struct ether_e_line,is_valid);
	dump_field(struct ether_e_line,index);
	dump_field(struct ether_e_line,label_to_push);
	dump_field(struct ether_e_line,NHLFE);
	dump_field(struct ether_e_line,e3iface);
	dump_field(struct ether_e_line,vlan_tci);
	dump_field(struct ether_e_line,ref_cnt);

	ck_assert(!!find_e_line_service(0));
	ck_assert(!!find_e_line_service(MAX_E_LINE_SERVICES-1));
	ck_assert(!find_e_line_service(MAX_E_LINE_SERVICES));

	struct ether_e_line eline;
	eline.e3iface=0;
	eline.vlan_tci=12;
	eline.label_to_push=0x322;
	eline.NHLFE=23;

	/*
	*maximum number of services supported
	*/
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		ck_assert(register_e_line_service(&eline)>=0);
		eline.vlan_tci++;
		eline.NHLFE++;
	}
	eline.vlan_tci++;
	eline.NHLFE++;
	ck_assert(register_e_line_service(&eline)<0);
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		e_line_base[idx].is_valid=0;
	}
	/*
	*element duplication
	*/
	ck_assert(register_e_line_service(&eline)>=0);
	ck_assert(register_e_line_service(&eline)<0);
	eline.vlan_tci++;
	ck_assert(register_e_line_service(&eline)<0);
	eline.NHLFE++;
	ck_assert(register_e_line_service(&eline)>=0);
	ck_assert(register_e_line_service(&eline)<0);

	/*
	*referenece count affairs
	*/

	ck_assert(reference_e_line_service(2)<0);
	ck_assert(!delete_e_line_service(0));
	
	ck_assert(!reference_e_line_service(1));
	ck_assert(delete_e_line_service(1)<0);
	ck_assert(!dereference_e_line_service(1));
	ck_assert(!delete_e_line_service(1));
	
}
END_TEST
ADD_TEST(leaf_e_service_general);
