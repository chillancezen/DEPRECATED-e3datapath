#include<stdint.h>
uint64_t leaf_api_list_csp_distribution_table(uint64_t * api_ret,
	uint16_t arg0,
	uint16_t arg1,
	uint8_t * arg2);
uint64_t leaf_api_csp_withdraw_port(uint64_t * api_ret,
	uint16_t arg3,
	uint16_t arg4);
uint64_t leaf_api_csp_setup_port(uint64_t * api_ret,
	uint16_t arg5,
	uint16_t arg6,
	uint8_t arg7,
	uint16_t arg8);
uint64_t leaf_api_delete_elan_fwd_entry(uint64_t * api_ret,
	uint16_t arg9,
	uint8_t * arg10);
uint64_t leaf_api_set_elan_fwd_entry(uint64_t * api_ret,
	uint16_t arg11,
	uint8_t * arg12,
	uint8_t arg13,
	uint16_t arg14,
	uint32_t arg15);
uint64_t leaf_api_reset_e_lan_multicast_fwd_entry(uint64_t * api_ret,
	uint16_t arg16);
uint64_t leaf_api_set_e_lan_multicast_fwd_entry(uint64_t * api_ret,
	uint16_t arg17,
	uint16_t arg18,
	uint32_t arg19);
uint64_t leaf_api_get_e_lan(uint64_t * api_ret,
	uint16_t arg20,
	uint8_t * arg21);
uint64_t leaf_api_delete_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg22,
	uint16_t arg23,
	uint16_t arg24);
uint64_t leaf_api_register_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg25,
	uint16_t arg26,
	uint32_t arg27,
	uint32_t arg28);
uint64_t leaf_api_delete_e_service_port(uint64_t * api_ret,
	uint8_t arg29,
	uint16_t arg30,
	uint16_t arg31);
uint64_t leaf_api_register_e_service_port(uint64_t * api_ret,
	uint8_t arg32,
	uint16_t arg33,
	uint32_t arg34,
	uint32_t arg35);
uint64_t leaf_api_delete_e_service(uint64_t * api_ret,
	uint8_t arg36,
	uint32_t arg37);
uint64_t leaf_api_list_e_services(uint64_t * api_ret,
	uint8_t arg38,
	uint8_t * arg39,
	uint8_t * arg40);
uint64_t leaf_api_get_e_line(uint64_t * api_ret,
	uint32_t arg41,
	uint8_t * arg42);
uint64_t leaf_api_register_e_service(uint64_t * api_ret,
	uint8_t arg43,
	uint8_t * arg44);
uint64_t leaf_api_clear_cbp_egress_nhlfe_index(uint64_t * api_ret,
	uint16_t arg45,
	uint32_t arg46);
uint64_t leaf_api_set_cbp_egress_nhlfe_index(uint64_t * api_ret,
	uint16_t arg47,
	uint32_t arg48,
	uint16_t arg49,
	uint32_t arg50);
uint64_t leaf_api_cbp_list_label_entry_partial(uint64_t * api_ret,
	uint16_t arg51,
	uint8_t * arg52,
	uint8_t * arg53,
	uint8_t * arg54,
	uint8_t * arg55);
uint64_t leaf_api_cbp_get_label_entry(uint64_t * api_ret,
	uint16_t arg56,
	uint32_t arg57,
	uint8_t * arg58);
uint64_t leaf_api_cbp_clear_label_entry(uint64_t * api_ret,
	uint16_t arg59,
	uint32_t arg60);
uint64_t leaf_api_cbp_setup_label_entry(uint64_t * api_ret,
	uint16_t arg61,
	uint32_t arg62,
	uint8_t arg63,
	uint16_t arg64);
uint64_t spine_api_register_or_delete_nexthop_in_mnexthop(uint64_t * api_ret,
	uint16_t arg65,
	uint8_t arg66,
	uint16_t arg67,
	uint32_t arg68);
uint64_t spine_api_delete_mnexthop(uint64_t * api_ret,
	uint16_t arg69);
uint64_t spine_api_list_mnexthops(uint64_t * api_ret,
	uint8_t * arg70,
	uint8_t * arg71);
uint64_t spine_api_get_mnexthop(uint64_t * api_ret,
	uint16_t arg72,
	uint8_t * arg73);
uint64_t spine_api_register_mnexthop(uint64_t * api_ret);
uint64_t spine_api_pbp_delete_label_entry(uint64_t * api_ret,
	uint16_t arg74,
	uint32_t arg75);
uint64_t cbp_api_list_spine_label_entry_partial(uint64_t * api_ret,
	uint16_t arg76,
	uint8_t * arg77,
	uint8_t * arg78,
	uint8_t * arg79,
	uint8_t * arg80);
uint64_t spine_api_pbp_get_label_entry(uint64_t * api_ret,
	uint16_t arg81,
	uint32_t arg82,
	uint8_t * arg83);
uint64_t spine_api_pbp_setup_label_entry(uint64_t * api_ret,
	uint16_t arg84,
	uint32_t arg85,
	uint8_t * arg86);
uint64_t update_e3fiace_status(uint64_t * api_ret,
	uint16_t arg87,
	uint8_t arg88);
uint64_t reclaim_e3iface(uint64_t * api_ret,
	uint16_t arg89);
uint64_t create_e3iface(uint64_t * api_ret,
	uint8_t * arg90,
	uint8_t arg91,
	uint8_t arg92,
	uint8_t arg93,
	uint8_t * arg94);
uint64_t e3net_api_delete_common_nexthop(uint64_t * api_ret,
	uint16_t arg95);
uint64_t e3net_api_list_common_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg96,
	uint8_t * arg97,
	uint8_t * arg98);
uint64_t e3net_api_get_common_nexthop(uint64_t * api_ret,
	uint16_t arg99,
	uint8_t * arg100);
uint64_t e3net_api_register_common_nexthop(uint64_t * api_ret,
	uint16_t arg101,
	uint16_t arg102);
uint64_t e3net_api_delete_common_neighbor(uint64_t * api_ret,
	uint16_t arg103);
uint64_t e3net_api_list_common_neighbor_partial(uint64_t * api_ret,
	uint8_t * arg104,
	uint8_t * arg105,
	uint8_t * arg106);
uint64_t e3net_api_get_common_neighbor(uint64_t * api_ret,
	uint16_t arg107,
	uint8_t * arg108);
uint64_t e3net_api_register_or_update_common_neighbor(uint64_t * api_ret,
	uint8_t arg109,
	uint8_t * arg110,
	uint8_t * arg111);
uint64_t dump_memory_structure(uint64_t * api_ret);
uint64_t e3datapath_version(uint64_t * api_ret);
uint64_t get_e3interface(uint64_t * api_ret,
	uint16_t arg112,
	uint8_t * arg113);
uint64_t list_e3interfaces(uint64_t * api_ret,
	uint8_t * arg114,
	uint8_t * arg115);
uint64_t infra_api_list_nodes(uint64_t * api_ret,
	uint8_t * arg116);
uint64_t infra_api_get_node(uint64_t * api_ret,
	uint16_t arg117,
	uint8_t * arg118);
