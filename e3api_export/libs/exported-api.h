#include<stdint.h>
uint64_t leaf_api_delete_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg0,
	uint16_t arg1,
	uint16_t arg2);
uint64_t leaf_api_register_e_service_nhlfe(uint64_t * api_ret,
	uint8_t arg3,
	uint16_t arg4,
	uint32_t arg5,
	uint32_t arg6);
uint64_t leaf_api_delete_e_service_port(uint64_t * api_ret,
	uint8_t arg7,
	uint16_t arg8,
	uint16_t arg9);
uint64_t leaf_api_register_e_service_port(uint64_t * api_ret,
	uint8_t arg10,
	uint16_t arg11,
	uint32_t arg12,
	uint32_t arg13);
uint64_t leaf_api_delete_e_service(uint64_t * api_ret,
	uint8_t arg14,
	uint32_t arg15);
uint64_t leaf_api_list_e_services(uint64_t * api_ret,
	uint8_t arg16,
	uint8_t * arg17,
	uint8_t * arg18);
uint64_t leaf_api_get_e_line(uint64_t * api_ret,
	uint32_t arg19,
	uint8_t * arg20);
uint64_t leaf_api_register_e_service(uint64_t * api_ret,
	uint8_t arg21,
	uint8_t * arg22);
uint64_t delete_label_entry(uint64_t * api_ret,
	uint16_t arg23,
	uint32_t arg24);
uint64_t list_label_entry_partial(uint64_t * api_ret,
	uint16_t arg25,
	uint8_t * arg26,
	uint8_t * arg27,
	uint8_t * arg28,
	uint8_t * arg29);
uint64_t get_label_entry(uint64_t * api_ret,
	uint16_t arg30,
	uint32_t arg31,
	uint8_t * arg32);
uint64_t register_label_entry(uint64_t * api_ret,
	uint16_t arg33,
	uint32_t arg34,
	uint8_t * arg35);
uint64_t delete_mnexthops(uint64_t * api_ret,
	uint16_t arg36);
uint64_t list_mnexthops_partial(uint64_t * api_ret,
	uint8_t * arg37,
	uint8_t * arg38,
	uint8_t * arg39,
	uint8_t * arg40);
uint64_t update_mnexthops(uint64_t * api_ret,
	uint16_t arg41,
	uint8_t * arg42);
uint64_t get_mnexthops(uint64_t * api_ret,
	uint16_t arg43,
	uint8_t * arg44);
uint64_t register_mnexthops(uint64_t * api_ret,
	uint8_t * arg45,
	uint8_t * arg46);
uint64_t delete_nexthop(uint64_t * api_ret,
	uint16_t arg47);
uint64_t list_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg48,
	uint8_t * arg49,
	uint8_t * arg50,
	uint8_t * arg51);
uint64_t get_nexthop(uint64_t * api_ret,
	uint16_t arg52,
	uint8_t * arg53);
uint64_t register_nexthop(uint64_t * api_ret,
	uint16_t arg54,
	uint8_t * arg55,
	uint8_t * arg56);
uint64_t delete_neighbour(uint64_t * api_ret,
	uint8_t * arg57);
uint64_t get_neighbour(uint64_t * api_ret,
	uint16_t arg58,
	uint8_t * arg59);
uint64_t list_neighbours_partial(uint64_t * api_ret,
	uint8_t * arg60,
	uint8_t * arg61,
	uint8_t * arg62,
	uint8_t * arg63);
uint64_t refresh_neighbor_mac(uint64_t * api_ret,
	uint8_t * arg64,
	uint8_t * arg65);
uint64_t register_neighbor(uint64_t * api_ret,
	uint8_t * arg66,
	uint8_t * arg67,
	uint8_t * arg68);
uint64_t update_e3fiace_status(uint64_t * api_ret,
	uint16_t arg69,
	uint8_t arg70);
uint64_t reclaim_e3iface(uint64_t * api_ret,
	uint16_t arg71);
uint64_t create_e3iface(uint64_t * api_ret,
	uint8_t * arg72,
	uint8_t arg73,
	uint8_t arg74,
	uint8_t arg75,
	uint8_t * arg76);
uint64_t e3net_api_delete_common_neighbor(uint64_t * api_ret,
	uint16_t arg77);
uint64_t e3net_api_list_common_neighbor_partial(uint64_t * api_ret,
	uint8_t * arg78,
	uint8_t * arg79,
	uint8_t * arg80);
uint64_t e3net_api_get_common_neighbor(uint64_t * api_ret,
	uint16_t arg81,
	uint8_t * arg82);
uint64_t e3net_api_register_or_update_common_neighbor(uint64_t * api_ret,
	uint8_t arg83,
	uint8_t * arg84,
	uint8_t * arg85);
uint64_t get_e3interface(uint64_t * api_ret,
	uint16_t arg86,
	uint8_t * arg87);
uint64_t list_e3interfaces(uint64_t * api_ret,
	uint8_t * arg88,
	uint8_t * arg89);
uint64_t e3datapath_version(uint64_t * api_ret);
