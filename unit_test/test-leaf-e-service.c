/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3test.h>
#include <leaf/include/leaf-e-service.h>
#include <e3infra/include/util.h>

DECLARE_TEST_CASE(tc_leaf_e_service);

START_TEST(leaf_e_line_service_general){
	int idx=0;
	#if 0
	int last_offset=0,last_size=0;
	printf("dump definition of ether_e_line:\n");
	dump_field(struct ether_e_line,is_valid);
	dump_field(struct ether_e_line,index);
	dump_field(struct ether_e_line,label_to_push);
	dump_field(struct ether_e_line,NHLFE);
	dump_field(struct ether_e_line,e3iface);
	dump_field(struct ether_e_line,vlan_tci);
	dump_field(struct ether_e_line,ref_cnt);
	#endif

	ck_assert(!!_find_e_line_service(0));
	ck_assert(!!_find_e_line_service(MAX_E_LINE_SERVICES-1));
	ck_assert(!_find_e_line_service(MAX_E_LINE_SERVICES));

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
	int eline_index=0;
	/*
	*maximum number of services supported
	*/
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		ck_assert((eline_index=register_e_line_service())>=0);
		ck_assert(!register_e_line_port(eline_index,eline.e3iface,eline.vlan_tci));
		ck_assert(!register_e_line_nhlfe(eline_index,eline.NHLFE,eline.label_to_push));
		eline.vlan_tci++;
		eline.label_to_push++;
	}
	
	eline.vlan_tci++;
	eline.label_to_push++;
	ck_assert((eline_index=register_e_line_service())<0);
	ck_assert(register_e_line_port(eline_index,eline.e3iface,eline.vlan_tci));
	ck_assert(register_e_line_nhlfe(eline_index,eline.NHLFE,eline.label_to_push));
	
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		e_line_base[idx].is_valid=0;
	}
	/*
	*element duplication
	*/
	ck_assert((eline_index=register_e_line_service())>=0);
	ck_assert(!register_e_line_port(eline_index,eline.e3iface,eline.vlan_tci));

	ck_assert((eline_index=register_e_line_service())>=0);
	ck_assert(register_e_line_port(eline_index,eline.e3iface,eline.vlan_tci));
	eline.vlan_tci++;
	ck_assert(!register_e_line_port(eline_index,eline.e3iface,eline.vlan_tci));
	ck_assert(!!find_e_line_service(eline_index));
	ck_assert(find_e_line_service(eline_index)->is_csp_ready);
	
	ck_assert((eline_index=register_e_line_service())>=0);
	ck_assert(!register_e_line_nhlfe(eline_index,eline.NHLFE,eline.label_to_push));

	ck_assert((eline_index=register_e_line_service())>=0);
	ck_assert(register_e_line_nhlfe(eline_index,eline.NHLFE,eline.label_to_push));
	eline.label_to_push++;
	ck_assert(!register_e_line_nhlfe(eline_index,eline.NHLFE,eline.label_to_push));
	ck_assert(!!find_e_line_service(eline_index));
	ck_assert(find_e_line_service(eline_index)->is_cbp_ready);


	ck_assert((eline_index=register_e_line_service())>=0);
	ck_assert(reference_e_line_service(MAX_E_LINE_SERVICES-1));
	ck_assert(reference_e_line_service(MAX_E_LINE_SERVICES));
	ck_assert(!reference_e_line_service(eline_index));

	ck_assert(delete_e_line_service(eline_index));
	ck_assert(!!find_e_line_service(eline_index));

	ck_assert(!dereference_e_line_service(eline_index));
	ck_assert(!delete_e_line_service(eline_index));
	ck_assert(!find_e_line_service(eline_index));
	
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
    int idx=0;
    /*
    *key operation test
    */
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
    /*
    *environmental pre-setup
    */
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
    
	struct ether_e_lan * elan;
	ck_assert(register_e_lan_service()==0);
	ck_assert(!!(elan=_find_e_lan_service(0)));
	ck_assert(elan->is_valid);

    /*
    *elan-ports related test
    */
    ck_assert(register_e_lan_port(0,0,0)==0);
    ck_assert(register_e_lan_port(0,1,0)==1);
    /*multiple interface with the same VLAN is allowed
    * user must not vswitch does not form a loop together with 
    *external swith in whatever cases,becuase vswitch does not know the exact
    *topology outside in customer's attachment circuit network
    *the best practice if a VXLAN port is attached, no VLAN is needed at all
    *if a VLAN network is attached to the vswith, for a certain vlan, it must be enforced that only one
    *port into the vswith,let external swith do the L2 switching before the traffic entres vswitch
    */
    ck_assert(register_e_lan_port(1,0,1)<0);
    ck_assert(register_e_lan_port(0,0,1)==2);
    ck_assert(register_e_lan_port(0,0,1)<0);
    ck_assert(elan->nr_ports==3);
    for(idx=3;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
        ck_assert(register_e_lan_port(0,idx,0)==idx);
    }
    ck_assert(register_e_lan_port(0,idx+1,0)<0);
    ck_assert(elan->nr_ports==MAX_PORTS_IN_E_LAN_SERVICE);
    for(idx=3;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
        ck_assert(find_e_lan_port(0,idx,0)==idx);
    }
    ck_assert(find_e_lan_port(0,idx+1,0)<0);
    for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
        ck_assert(!delete_e_lan_port(0,idx));
    }
    ck_assert(!elan->nr_ports);
    /*
    *elan-nhlfe
    */
    ck_assert(register_e_lan_nhlfe(0,1,0x123)<0);
    ck_assert(register_e_lan_nhlfe(1,0,0x123)<0);
    ck_assert(register_e_lan_nhlfe(0,0,0x123)==0);
    ck_assert(register_e_lan_nhlfe(0,0,0x123)<0);
    ck_assert(register_e_lan_nhlfe(0,0,0x124)==1);
    for(idx=2;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
        ck_assert(register_e_lan_nhlfe(0,0,idx)==idx);
    }
    for(idx=2;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
        ck_assert(find_e_lan_nhlfe(0,0,idx)==idx);
    }
    ck_assert(register_e_lan_nhlfe(0,0,idx)<0);
    ck_assert(elan->nr_nhlfes==MAX_NHLFE_IN_E_LAN_SERVICE);
    for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
        ck_assert(!delete_e_lan_nhlfe(0,idx));
    }
    ck_assert(!elan->nr_nhlfes);

    /*
    *port fwd entry of e-lan service
    */
    uint8_t mac2[6];
    mac2[0]=0x12;
    mac2[1]=0x35;
    mac2[2]=0xfe;
    mac2[3]=0xf3;
    mac2[4]=0x00;
    mac2[5]=0x36;
    struct findex_2_4_key key1;
    struct e_lan_fwd_entry fwd_entry;
    fwd_entry.is_port_entry=1;
    fwd_entry.e3iface=2;
    fwd_entry.vlan_tci=0x123;
	ck_assert(register_e_lan_fwd_entry(1, mac2,&fwd_entry)<0);
    ck_assert(register_e_lan_fwd_entry(0, mac2,&fwd_entry)<0);
    ck_assert(register_e_lan_port(0,2,0x124)==0);
	
	ck_assert(register_e_lan_fwd_entry(0, mac2,&fwd_entry)<0);
	fwd_entry.vlan_tci=0x124;
	mac_to_findex_2_4_key(mac2,&key1);
	ck_assert(!elan->fib_base[key1.key_index].next);
	/*12:35:fe:f3:00:36-->port(2,0x123)*/
    ck_assert(!register_e_lan_fwd_entry(0, mac2,&fwd_entry));
	ck_assert(!!elan->fib_base[key1.key_index].next);
	
	mac_to_findex_2_4_key(mac2,&key1);
	ck_assert(!fast_index_2_4_item_safe(elan->fib_base,&key1));
	ck_assert(key1.value_as_u64==fwd_entry.entry_as_u64);
	
	mac2[5]=0x37;
	mac_to_findex_2_4_key(mac2,&key1);
	ck_assert(fast_index_2_4_item_safe(elan->fib_base,&key1));

	mac2[0]=0x12;
    mac2[1]=0x36;
    mac2[2]=0xfe;
    mac2[3]=0xf3;
    mac2[4]=0x00;
    mac2[5]=0x36;
	fwd_entry.is_port_entry=1;
    fwd_entry.e3iface=2;
    fwd_entry.vlan_tci=0x124;
	ck_assert(!register_e_lan_fwd_entry(0, mac2,&fwd_entry));

    mac2[1]=0x35;
	mac_to_findex_2_4_key(mac2,&key1);
	ck_assert(!delete_e_lan_fwd_entry(0,mac2));
	ck_assert(fast_index_2_4_item_safe(elan->fib_base,&key1));
	ck_assert(!!elan->fib_base[key1.key_index].next);
	mac2[1]=0x36;
	mac_to_findex_2_4_key(mac2,&key1);
	ck_assert(!delete_e_lan_fwd_entry(0,mac2));
	ck_assert(fast_index_2_4_item_safe(elan->fib_base,&key1));
	ck_assert(!elan->fib_base[key1.key_index].next);
    
    mac2[0]=0x12;
    mac2[1]=0x36;
    mac2[2]=0xfe;
    mac2[3]=0xf3;
    mac2[4]=0x00;
    mac2[5]=0x36;
    fwd_entry.is_port_entry=1;
    fwd_entry.e3iface=2;
    fwd_entry.vlan_tci=0x124;
    mac_to_findex_2_4_key(mac2,&key1);
    for(idx=0;idx<FINDEX_2_1_6_TAGS_LENGTH;idx++){
        mac2[0]=idx;
        ck_assert(!register_e_lan_fwd_entry(0,mac2,&fwd_entry));
    }
    ck_assert(!!elan->fib_base[key1.key_index].next);
    ck_assert(!elan->fib_base[key1.key_index].next->next_entry);
    for(idx=FINDEX_2_1_6_TAGS_LENGTH;idx<FINDEX_2_1_6_TAGS_LENGTH*2;idx++){
        mac2[0]=idx;
        ck_assert(!register_e_lan_fwd_entry(0,mac2,&fwd_entry));
    }
    ck_assert(!!elan->fib_base[key1.key_index].next->next_entry);
    ck_assert(!elan->fib_base[key1.key_index].next->next_entry->next_entry);

    for(idx=FINDEX_2_1_6_TAGS_LENGTH;idx<FINDEX_2_1_6_TAGS_LENGTH*2;idx++){
        mac2[0]=idx;
        ck_assert(!delete_e_lan_fwd_entry(0,mac2));
    }
    ck_assert(!elan->fib_base[key1.key_index].next->next_entry);
    for(idx=0;idx<FINDEX_2_1_6_TAGS_LENGTH;idx++){
        mac2[0]=idx;
        ck_assert(!delete_e_lan_fwd_entry(0,mac2));
    }
    ck_assert(!elan->fib_base[key1.key_index].next);

    /*
    *nhlfe fwd entry of e-lan service
    */
    mac2[0]=0x12;
    mac2[1]=0x36;
    mac2[2]=0xfe;
    mac2[3]=0xf3;
    mac2[4]=0x00;
    mac2[5]=0x36;
    fwd_entry.is_port_entry=0;
    fwd_entry.NHLFE=0;/*in this test case, only 0th NHLFE is valid*/
    fwd_entry.label_to_push=0x234;
	mac_to_findex_2_4_key(mac2,&key1);
    ck_assert(register_e_lan_fwd_entry(0,mac2,&fwd_entry));
    ck_assert(register_e_lan_nhlfe(0,0,0x234)==0);
	ck_assert(!register_e_lan_fwd_entry(0,mac2,&fwd_entry));

	
	ck_assert(!!elan->fib_base[key1.key_index].next);
	ck_assert(!fast_index_2_4_item_safe(elan->fib_base,&key1));
	ck_assert(fwd_entry.entry_as_u64==key1.value_as_u64);

	ck_assert(!delete_e_lan_fwd_entry(0,mac2));
	ck_assert(fast_index_2_4_item_safe(elan->fib_base,&key1));
	ck_assert(!elan->fib_base[key1.key_index].next);

	for(idx=0;idx<FINDEX_2_1_6_TAGS_LENGTH;idx++){
        mac2[0]=idx;
        ck_assert(!register_e_lan_fwd_entry(0,mac2,&fwd_entry));
    }
	ck_assert(!elan->fib_base[key1.key_index].next->next_entry);

	for(idx=FINDEX_2_1_6_TAGS_LENGTH;idx<FINDEX_2_1_6_TAGS_LENGTH*2;idx++){
        mac2[0]=idx;
        ck_assert(!register_e_lan_fwd_entry(0,mac2,&fwd_entry));
    }
    ck_assert(!!elan->fib_base[key1.key_index].next->next_entry);
    ck_assert(!elan->fib_base[key1.key_index].next->next_entry->next_entry);

	for(idx=FINDEX_2_1_6_TAGS_LENGTH;idx<FINDEX_2_1_6_TAGS_LENGTH*2;idx++){
        mac2[0]=idx;
        ck_assert(!delete_e_lan_fwd_entry(0,mac2));
    }
    ck_assert(!elan->fib_base[key1.key_index].next->next_entry);
    for(idx=0;idx<FINDEX_2_1_6_TAGS_LENGTH;idx++){
        mac2[0]=idx;
        ck_assert(!delete_e_lan_fwd_entry(0,mac2));
    }
    ck_assert(!elan->fib_base[key1.key_index].next);
	ck_assert(!delete_e_lan_port(0,0));
	ck_assert(!delete_e_lan_nhlfe(0,0));
	ck_assert(!elan->nr_ports);
	ck_assert(!elan->nr_nhlfes);
	
	/*
	*elan-port & elan-nhlfe deletion test
	*/
	mac2[0]=0x12;
    mac2[1]=0x36;
    mac2[2]=0xfe;
    mac2[3]=0xf3;
    mac2[4]=0x00;
    mac2[5]=0x36;
	fwd_entry.is_port_entry=1;
    fwd_entry.e3iface=1;
    fwd_entry.vlan_tci=0x123;
	ck_assert(register_e_lan_port(0,1,0x123)>=0);
	ck_assert(elan->nr_ports==1);
	ck_assert(!register_e_lan_fwd_entry(0,mac2,&fwd_entry));
	mac_to_findex_2_4_key(mac2,&key1);
	ck_assert(!fast_index_2_4_item_safe(elan->fib_base,&key1));
	ck_assert(!delete_e_lan_port(0,0));
	ck_assert(elan->nr_ports==0);
	ck_assert(fast_index_2_4_item_safe(elan->fib_base,&key1));

	fwd_entry.is_port_entry=0;
    fwd_entry.e3iface=0;
    fwd_entry.vlan_tci=0x234;
	ck_assert(register_e_lan_nhlfe(0,0,0x234)>=0);
	ck_assert(elan->nr_nhlfes==1);
	ck_assert(!register_e_lan_fwd_entry(0,mac2,&fwd_entry));
	mac_to_findex_2_4_key(mac2,&key1);
	ck_assert(!fast_index_2_4_item_safe(elan->fib_base,&key1));
	ck_assert(!delete_e_lan_nhlfe(0,0));
	ck_assert(elan->nr_nhlfes==0);
	ck_assert(fast_index_2_4_item_safe(elan->fib_base,&key1));

	ck_assert(reference_e_lan_service(MAX_E_LAN_SERVICES));
	ck_assert(reference_e_lan_service(MAX_E_LAN_SERVICES-1));
	ck_assert(!reference_e_lan_service(0));
	ck_assert(delete_e_lan_service(0));
	ck_assert(dereference_e_lan_service(MAX_E_LAN_SERVICES));
	ck_assert(dereference_e_lan_service(MAX_E_LAN_SERVICES-1));
	ck_assert(!dereference_e_lan_service(0));
	ck_assert(!delete_e_lan_service(0));
}
END_TEST
ADD_TEST(leaf_e_lan_service_general);

