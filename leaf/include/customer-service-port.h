/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _CUSTOMER_SERVICE_PORT_H
#define _CUSTOMER_SERVICE_PORT_H
#include <stdint.h>
#include <rte_rwlock.h>

struct csp_private{

	rte_rwlock_t csp_guard;
	__attribute__((packed))
		struct{
			uint8_t is_valid;
			uint8_t e_service;
			uint16_t service_index;
		}vlans[4096];
	#if 0
	uint32_t attached;/*whether attached to an ethernet service*/
	uint32_t e_service;
	uint32_t service_index;
	uint32_t vlan_tci;
	#endif
};
#define CSP_NODE_BURST_SIZE 48

#define CSP_CACHE_SIZE 8
#define CSP_CACHE_MASK (CSP_CACHE_SIZE-1)

#define CSP_MAC_CACHE_SIZE 8
#define CSP_MAC_CACHE_MASK (CSP_MAC_CACHE_SIZE-1)


#define CSP_PROCESS_INPUT_DROP 0x0
#define CSP_PROCESS_INPUT_ELINE_FWD 0x1
#define CSP_PROCESS_INPUT_ELAN_UNICAST_FWD 0x2
#define CSP_PROCESS_INPUT_ELAN_MULTICAST_FWD 0x3
#endif