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
uint64_t leaf_api_get_e_lan(uint64_t * api_ret,
	uint16_t arg9,
	uint8_t * arg10);
uint64_t leaf_api_delete_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg11,
	uint16_t arg12,
	uint16_t arg13);
uint64_t leaf_api_register_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg14,
	uint16_t arg15,
	uint32_t arg16,
	uint32_t arg17);
uint64_t leaf_api_delete_e_service_port(uint64_t * api_ret,
	uint8_t arg18,
	uint16_t arg19,
	uint16_t arg20);
uint64_t leaf_api_register_e_service_port(uint64_t * api_ret,
	uint8_t arg21,
	uint16_t arg22,
	uint32_t arg23,
	uint32_t arg24);
uint64_t leaf_api_delete_e_service(uint64_t * api_ret,
	uint8_t arg25,
	uint32_t arg26);
uint64_t leaf_api_list_e_services(uint64_t * api_ret,
	uint8_t arg27,
	uint8_t * arg28,
	uint8_t * arg29);
uint64_t leaf_api_get_e_line(uint64_t * api_ret,
	uint32_t arg30,
	uint8_t * arg31);
uint64_t leaf_api_register_e_service(uint64_t * api_ret,
	uint8_t arg32,
	uint8_t * arg33);
uint64_t leaf_api_clear_cbp_egress_nhlfe_index(uint64_t * api_ret,
	uint16_t arg34,
	uint32_t arg35);
uint64_t leaf_api_set_cbp_egress_nhlfe_index(uint64_t * api_ret,
	uint16_t arg36,
	uint32_t arg37,
	uint16_t arg38,
	uint32_t arg39);
uint64_t leaf_api_cbp_list_label_entry_partial(uint64_t * api_ret,
	uint16_t arg40,
	uint8_t * arg41,
	uint8_t * arg42,
	uint8_t * arg43,
	uint8_t * arg44);
uint64_t leaf_api_cbp_get_label_entry(uint64_t * api_ret,
	uint16_t arg45,
	uint32_t arg46,
	uint8_t * arg47);
uint64_t leaf_api_cbp_clear_label_entry(uint64_t * api_ret,
	uint16_t arg48,
	uint32_t arg49);
uint64_t leaf_api_cbp_setup_label_entry(uint64_t * api_ret,
	uint16_t arg50,
	uint32_t arg51,
	uint8_t arg52,
	uint16_t arg53);
uint64_t spine_api_register_or_delete_nexthop_in_mnexthop(uint64_t * api_ret,
	uint16_t arg54,
	uint8_t arg55,
	uint16_t arg56,
	uint32_t arg57);
uint64_t spine_api_delete_mnexthop(uint64_t * api_ret,
	uint16_t arg58);
uint64_t spine_api_list_mnexthops(uint64_t * api_ret,
	uint8_t * arg59,
	uint8_t * arg60);
uint64_t spine_api_get_mnexthop(uint64_t * api_ret,
	uint16_t arg61,
	uint8_t * arg62);
uint64_t spine_api_register_mnexthop(uint64_t * api_ret);
uint64_t spine_api_pbp_delete_label_entry(uint64_t * api_ret,
	uint16_t arg63,
	uint32_t arg64);
uint64_t cbp_api_list_spine_label_entry_partial(uint64_t * api_ret,
	uint16_t arg65,
	uint8_t * arg66,
	uint8_t * arg67,
	uint8_t * arg68,
	uint8_t * arg69);
uint64_t spine_api_pbp_get_label_entry(uint64_t * api_ret,
	uint16_t arg70,
	uint32_t arg71,
	uint8_t * arg72);
uint64_t spine_api_pbp_setup_label_entry(uint64_t * api_ret,
	uint16_t arg73,
	uint32_t arg74,
	uint8_t * arg75);
uint64_t update_e3fiace_status(uint64_t * api_ret,
	uint16_t arg76,
	uint8_t arg77);
uint64_t reclaim_e3iface(uint64_t * api_ret,
	uint16_t arg78);
uint64_t create_e3iface(uint64_t * api_ret,
	uint8_t * arg79,
	uint8_t arg80,
	uint8_t arg81,
	uint8_t arg82,
	uint8_t * arg83);
uint64_t e3net_api_delete_common_nexthop(uint64_t * api_ret,
	uint16_t arg84);
uint64_t e3net_api_list_common_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg85,
	uint8_t * arg86,
	uint8_t * arg87);
uint64_t e3net_api_get_common_nexthop(uint64_t * api_ret,
	uint16_t arg88,
	uint8_t * arg89);
uint64_t e3net_api_register_common_nexthop(uint64_t * api_ret,
	uint16_t arg90,
	uint16_t arg91);
uint64_t e3net_api_delete_common_neighbor(uint64_t * api_ret,
	uint16_t arg92);
uint64_t e3net_api_list_common_neighbor_partial(uint64_t * api_ret,
	uint8_t * arg93,
	uint8_t * arg94,
	uint8_t * arg95);
uint64_t e3net_api_get_common_neighbor(uint64_t * api_ret,
	uint16_t arg96,
	uint8_t * arg97);
uint64_t e3net_api_register_or_update_common_neighbor(uint64_t * api_ret,
	uint8_t arg98,
	uint8_t * arg99,
	uint8_t * arg100);
uint64_t get_e3interface(uint64_t * api_ret,
	uint16_t arg101,
	uint8_t * arg102);
uint64_t list_e3interfaces(uint64_t * api_ret,
	uint8_t * arg103,
	uint8_t * arg104);
uint64_t e3datapath_version(uint64_t * api_ret);
