#include<stdint.h>
uint64_t delete_label_entry(uint64_t * api_ret,
	uint16_t arg0,
	uint32_t arg1);
uint64_t list_label_entry_partial(uint64_t * api_ret,
	uint16_t arg2,
	uint8_t * arg3,
	uint8_t * arg4,
	uint8_t * arg5,
	uint8_t * arg6);
uint64_t get_label_entry(uint64_t * api_ret,
	uint16_t arg7,
	uint32_t arg8,
	uint8_t * arg9);
uint64_t register_label_entry(uint64_t * api_ret,
	uint16_t arg10,
	uint32_t arg11,
	uint8_t * arg12);
uint64_t delete_mnexthops(uint64_t * api_ret,
	uint16_t arg13);
uint64_t list_mnexthops_partial(uint64_t * api_ret,
	uint8_t * arg14,
	uint8_t * arg15,
	uint8_t * arg16,
	uint8_t * arg17);
uint64_t update_mnexthops(uint64_t * api_ret,
	uint16_t arg18,
	uint8_t * arg19);
uint64_t get_mnexthops(uint64_t * api_ret,
	uint16_t arg20,
	uint8_t * arg21);
uint64_t register_mnexthops(uint64_t * api_ret,
	uint8_t * arg22,
	uint8_t * arg23);
uint64_t delete_nexthop(uint64_t * api_ret,
	uint16_t arg24);
uint64_t list_nexthop_partial(uint64_t * api_ret,
	uint8_t * arg25,
	uint8_t * arg26,
	uint8_t * arg27,
	uint8_t * arg28);
uint64_t get_nexthop(uint64_t * api_ret,
	uint16_t arg29,
	uint8_t * arg30);
uint64_t register_nexthop(uint64_t * api_ret,
	uint16_t arg31,
	uint8_t * arg32,
	uint8_t * arg33);
uint64_t delete_neighbour(uint64_t * api_ret,
	uint8_t * arg34);
uint64_t get_neighbour(uint64_t * api_ret,
	uint16_t arg35,
	uint8_t * arg36);
uint64_t list_neighbours_partial(uint64_t * api_ret,
	uint8_t * arg37,
	uint8_t * arg38,
	uint8_t * arg39,
	uint8_t * arg40);
uint64_t refresh_neighbor_mac(uint64_t * api_ret,
	uint8_t * arg41,
	uint8_t * arg42);
uint64_t register_neighbor(uint64_t * api_ret,
	uint8_t * arg43,
	uint8_t * arg44,
	uint8_t * arg45);
uint64_t update_e3fiace_status(uint64_t * api_ret,
	uint16_t arg46,
	uint8_t arg47);
uint64_t reclaim_e3iface(uint64_t * api_ret,
	uint16_t arg48);
uint64_t create_e3iface(uint64_t * api_ret,
	uint8_t * arg49,
	uint8_t arg50,
	uint8_t arg51,
	uint8_t * arg52);
uint64_t get_e3interface(uint64_t * api_ret,
	uint16_t arg53,
	uint8_t * arg54);
uint64_t list_e3interfaces(uint64_t * api_ret,
	uint8_t * arg55,
	uint8_t * arg56);
uint64_t e3datapath_version(uint64_t * api_ret);
