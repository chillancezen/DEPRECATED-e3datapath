#ifndef _COMMON_CACHE_H
#define _COMMON_CACHE_H
#include <stdint.h>
#include <leaf/include/leaf-e-service.h>
#include <leaf/include/leaf-label-fib.h>
#include <e3interface.h>

struct cbp_cache_entry{
	uint32_t is_valid;
	uint32_t label; 
	struct leaf_label_entry * lentry;
	struct ether_e_lan * elan;
	struct ether_e_line * eline;
}__attribute__((aligned(8)));

struct mac_cache_entry{
	uint8_t mac[6];
	uint8_t is_valid;
	uint8_t reserved0;
	struct e_lan_fwd_entry fwd_entry;
	#if defined(__CONTEXT_CUSTOEMR_SERVICE_PORT)
		struct common_nexthop * nexthop;
		struct common_neighbor* neighbor;
		struct E3Interface    * src_pif;
	#endif
}__attribute__((aligned(8)));

struct csp_cache_entry{
	uint16_t is_valid;
	uint16_t vlan_tci;
	uint32_t is_e_line;
	struct ether_e_lan * elan;
	struct ether_e_line * eline;

	/*e_line specific fields*/
	struct common_nexthop *	eline_nexthop;
	struct E3Interface	  *	eline_src_pif;
	struct common_neighbor*	eline_neighbor;
};

#endif
