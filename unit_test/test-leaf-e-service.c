#include <e3test.h>
#include <leaf/include/leaf-e-service.h>
#include <util.h>

DECLARE_TEST_CASE(tc_leaf_e_service);

START_TEST(leaf_e_line_service_general){
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

	/*
	*environmental presetup
	*/
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		e_line_base[idx].is_valid=0;
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
	
	struct ether_e_line eline;
	eline.e3iface=0;
	eline.vlan_tci=12;
	eline.label_to_push=0x322;
	eline.NHLFE=0;

	/*
	*maximum number of services supported
	*/
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		ck_assert(register_e_line_service(&eline)>=0);
		eline.vlan_tci++;
		eline.label_to_push++;
	}
	eline.vlan_tci++;
	eline.label_to_push++;
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
	eline.label_to_push++;
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

	/*
	*environmental cleanup
	*/
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		e_line_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		neighbor_base[idx].is_valid=0;
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		nexthop_base[idx].is_valid=0;
}
END_TEST
ADD_TEST(leaf_e_line_service_general);

START_TEST(leaf_e_lan_service_general){
    struct findex_2_4_key key;
    uint8_t mac[6],mac1[6];
    mac[0]=0x3f;
    mac[1]=0xfe;
    mac[2]=0x10;
    mac[3]=0x1;
    mac[4]=0x2;
    mac[5]=0x3e;
    mac_to_findex_2_4_key(mac, &key);
    findex_2_4_key_to_mac(&key, mac1);
    ck_assert(mac[0]==mac1[0]);
    ck_assert(mac[1]==mac1[1]);
    ck_assert(mac[2]==mac1[2]);
    ck_assert(mac[3]==mac1[3]);
    ck_assert(mac[4]==mac1[4]);
    ck_assert(mac[5]==mac1[5]);
    
	struct ether_e_lan * elan;
	ck_assert(register_e_lan_service()==0);
	ck_assert(!!(elan=find_e_lan_service(0)));
	ck_assert(elan->is_valid);
}
END_TEST
ADD_TEST(leaf_e_lan_service_general);

