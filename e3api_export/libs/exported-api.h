#include<stdint.h>
uint64_t list_neighbours_partial(uint64_t * api_ret,
	uint8_t * arg0,
	uint8_t * arg1,
	uint8_t * arg2);
uint64_t refresh_neighbor_mac(uint64_t * api_ret,
	uint8_t * arg3,
	uint8_t * arg4);
uint64_t register_neighbor(uint64_t * api_ret,
	uint8_t * arg5,
	uint8_t * arg6,
	uint8_t * arg7);
uint64_t update_e3fiace_status(uint64_t * api_ret,
	uint16_t arg8,
	uint8_t arg9);
uint64_t reclaim_e3iface(uint64_t * api_ret,
	uint16_t arg10);
uint64_t create_e3iface(uint64_t * api_ret,
	uint8_t * arg11,
	uint8_t arg12,
	uint8_t arg13,
	uint8_t * arg14);
uint64_t get_e3interface(uint64_t * api_ret,
	uint16_t arg15,
	uint8_t * arg16);
uint64_t list_e3interfaces(uint64_t * api_ret,
	uint8_t * arg17,
	uint8_t * arg18);
uint64_t e3datapath_version(uint64_t * api_ret);
