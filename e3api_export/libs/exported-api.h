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
uint64_t leaf_api_cbp_clear_label_entry(uint64_t * api_ret,
	uint16_t arg34,
	uint32_t arg35);
uint64_t leaf_api_cbp_setup_label_entry(uint64_t * api_ret,
	uint16_t arg36,
	uint32_t arg37,
	uint8_t arg38,
	uint16_t arg39);
uint64_t delete_label_entry(uint64_t * api_ret,
	uint16_t arg40,
	uint32_t arg41);
uint64_t list_label_entry_partial(uint64_t * api_ret,
	uint16_t arg42,
	uint8_t * arg43,
	uint8_t * arg44,
	uint8_t * arg45,
	uint8_t * arg46);
uint64_t get_label_entry(uint64_t * api_ret,
	uint16_t arg47,
	uint32_t arg48,
	uint8_t * arg49);
uint64_t register_label_entry(uint64_t * api_ret,
	uint16_t arg50,
	uint32_t arg51,
	uint8_t * arg52);
uint64_t delete_mnexthops(uint64_t * api_ret,
	uint16_t arg53);
uint64_t list_mnexthops_partial(uint64_t * api_ret,
	uint8_t * arg54,
	uint8_t * arg55,
	uint8_t * arg56,
	uint8_t * arg57);
uint64_t update_mnexthops(uint64_t * api_ret,
	uint16_t arg58,
	uint8_t * arg59);
uint64_t get_mnexthops(uint64_t * api_ret,
	uint16_t arg60,
	uint8_t * arg61);
uint64_t register_mnexthops(uint64_t * api_ret,
	uint8_t * arg62,
	uint8_t * arg63);
uint64_t delete_nexthop(uint64_t * api_ret,
	uint16_t arg64);
uint64_t list_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg65,
	uint8_t * arg66,
	uint8_t * arg67,
	uint8_t * arg68);
uint64_t get_nexthop(uint64_t * api_ret,
	uint16_t arg69,
	uint8_t * arg70);
uint64_t register_nexthop(uint64_t * api_ret,
	uint16_t arg71,
	uint8_t * arg72,
	uint8_t * arg73);
uint64_t delete_neighbour(uint64_t * api_ret,
	uint8_t * arg74);
uint64_t get_neighbour(uint64_t * api_ret,
	uint16_t arg75,
	uint8_t * arg76);
uint64_t list_neighbours_partial(uint64_t * api_ret,
	uint8_t * arg77,
	uint8_t * arg78,
	uint8_t * arg79,
	uint8_t * arg80);
uint64_t refresh_neighbor_mac(uint64_t * api_ret,
	uint8_t * arg81,
	uint8_t * arg82);
uint64_t register_neighbor(uint64_t * api_ret,
	uint8_t * arg83,
	uint8_t * arg84,
	uint8_t * arg85);
uint64_t update_e3fiace_status(uint64_t * api_ret,
	uint16_t arg86,
	uint8_t arg87);
uint64_t reclaim_e3iface(uint64_t * api_ret,
	uint16_t arg88);
uint64_t create_e3iface(uint64_t * api_ret,
	uint8_t * arg89,
	uint8_t arg90,
	uint8_t arg91,
	uint8_t arg92,
	uint8_t * arg93);
uint64_t e3net_api_delete_common_nexthop(uint64_t * api_ret,
	uint16_t arg94);
uint64_t e3net_api_list_common_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg95,
	uint8_t * arg96,
	uint8_t * arg97);
uint64_t e3net_api_get_common_nexthop(uint64_t * api_ret,
	uint16_t arg98,
	uint8_t * arg99);
uint64_t e3net_api_register_common_nexthop(uint64_t * api_ret,
	uint16_t arg100,
	uint16_t arg101);
uint64_t e3net_api_delete_common_neighbor(uint64_t * api_ret,
	uint16_t arg102);
uint64_t e3net_api_list_common_neighbor_partial(uint64_t * api_ret,
	uint8_t * arg103,
	uint8_t * arg104,
	uint8_t * arg105);
uint64_t e3net_api_get_common_neighbor(uint64_t * api_ret,
	uint16_t arg106,
	uint8_t * arg107);
uint64_t e3net_api_register_or_update_common_neighbor(uint64_t * api_ret,
	uint8_t arg108,
	uint8_t * arg109,
	uint8_t * arg110);
uint64_t get_e3interface(uint64_t * api_ret,
	uint16_t arg111,
	uint8_t * arg112);
uint64_t list_e3interfaces(uint64_t * api_ret,
	uint8_t * arg113,
	uint8_t * arg114);
uint64_t e3datapath_version(uint64_t * api_ret);
