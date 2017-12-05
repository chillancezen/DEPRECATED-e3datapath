#include<stdint.h>
uint64_t leaf_api_csp_setup_port(uint64_t * api_ret,
	uint16_t arg0,
	uint16_t arg1,
	uint8_t arg2,
	uint16_t arg3);
uint64_t leaf_api_get_e_lan(uint64_t * api_ret,
	uint16_t arg4,
	uint8_t * arg5);
uint64_t leaf_api_delete_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg6,
	uint16_t arg7,
	uint16_t arg8);
uint64_t leaf_api_register_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg9,
	uint16_t arg10,
	uint32_t arg11,
	uint32_t arg12);
uint64_t leaf_api_delete_e_service_port(uint64_t * api_ret,
	uint8_t arg13,
	uint16_t arg14,
	uint16_t arg15);
uint64_t leaf_api_register_e_service_port(uint64_t * api_ret,
	uint8_t arg16,
	uint16_t arg17,
	uint32_t arg18,
	uint32_t arg19);
uint64_t leaf_api_delete_e_service(uint64_t * api_ret,
	uint8_t arg20,
	uint32_t arg21);
uint64_t leaf_api_list_e_services(uint64_t * api_ret,
	uint8_t arg22,
	uint8_t * arg23,
	uint8_t * arg24);
uint64_t leaf_api_get_e_line(uint64_t * api_ret,
	uint32_t arg25,
	uint8_t * arg26);
uint64_t leaf_api_register_e_service(uint64_t * api_ret,
	uint8_t arg27,
	uint8_t * arg28);
uint64_t delete_label_entry(uint64_t * api_ret,
	uint16_t arg29,
	uint32_t arg30);
uint64_t list_label_entry_partial(uint64_t * api_ret,
	uint16_t arg31,
	uint8_t * arg32,
	uint8_t * arg33,
	uint8_t * arg34,
	uint8_t * arg35);
uint64_t get_label_entry(uint64_t * api_ret,
	uint16_t arg36,
	uint32_t arg37,
	uint8_t * arg38);
uint64_t register_label_entry(uint64_t * api_ret,
	uint16_t arg39,
	uint32_t arg40,
	uint8_t * arg41);
uint64_t delete_mnexthops(uint64_t * api_ret,
	uint16_t arg42);
uint64_t list_mnexthops_partial(uint64_t * api_ret,
	uint8_t * arg43,
	uint8_t * arg44,
	uint8_t * arg45,
	uint8_t * arg46);
uint64_t update_mnexthops(uint64_t * api_ret,
	uint16_t arg47,
	uint8_t * arg48);
uint64_t get_mnexthops(uint64_t * api_ret,
	uint16_t arg49,
	uint8_t * arg50);
uint64_t register_mnexthops(uint64_t * api_ret,
	uint8_t * arg51,
	uint8_t * arg52);
uint64_t delete_nexthop(uint64_t * api_ret,
	uint16_t arg53);
uint64_t list_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg54,
	uint8_t * arg55,
	uint8_t * arg56,
	uint8_t * arg57);
uint64_t get_nexthop(uint64_t * api_ret,
	uint16_t arg58,
	uint8_t * arg59);
uint64_t register_nexthop(uint64_t * api_ret,
	uint16_t arg60,
	uint8_t * arg61,
	uint8_t * arg62);
uint64_t delete_neighbour(uint64_t * api_ret,
	uint8_t * arg63);
uint64_t get_neighbour(uint64_t * api_ret,
	uint16_t arg64,
	uint8_t * arg65);
uint64_t list_neighbours_partial(uint64_t * api_ret,
	uint8_t * arg66,
	uint8_t * arg67,
	uint8_t * arg68,
	uint8_t * arg69);
uint64_t refresh_neighbor_mac(uint64_t * api_ret,
	uint8_t * arg70,
	uint8_t * arg71);
uint64_t register_neighbor(uint64_t * api_ret,
	uint8_t * arg72,
	uint8_t * arg73,
	uint8_t * arg74);
uint64_t update_e3fiace_status(uint64_t * api_ret,
	uint16_t arg75,
	uint8_t arg76);
uint64_t reclaim_e3iface(uint64_t * api_ret,
	uint16_t arg77);
uint64_t create_e3iface(uint64_t * api_ret,
	uint8_t * arg78,
	uint8_t arg79,
	uint8_t arg80,
	uint8_t arg81,
	uint8_t * arg82);
uint64_t e3net_api_delete_common_nexthop(uint64_t * api_ret,
	uint16_t arg83);
uint64_t e3net_api_list_common_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg84,
	uint8_t * arg85,
	uint8_t * arg86);
uint64_t e3net_api_get_common_nexthop(uint64_t * api_ret,
	uint16_t arg87,
	uint8_t * arg88);
uint64_t e3net_api_register_common_nexthop(uint64_t * api_ret,
	uint16_t arg89,
	uint16_t arg90);
uint64_t e3net_api_delete_common_neighbor(uint64_t * api_ret,
	uint16_t arg91);
uint64_t e3net_api_list_common_neighbor_partial(uint64_t * api_ret,
	uint8_t * arg92,
	uint8_t * arg93,
	uint8_t * arg94);
uint64_t e3net_api_get_common_neighbor(uint64_t * api_ret,
	uint16_t arg95,
	uint8_t * arg96);
uint64_t e3net_api_register_or_update_common_neighbor(uint64_t * api_ret,
	uint8_t arg97,
	uint8_t * arg98,
	uint8_t * arg99);
uint64_t get_e3interface(uint64_t * api_ret,
	uint16_t arg100,
	uint8_t * arg101);
uint64_t list_e3interfaces(uint64_t * api_ret,
	uint8_t * arg102,
	uint8_t * arg103);
uint64_t e3datapath_version(uint64_t * api_ret);
