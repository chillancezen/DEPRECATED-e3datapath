/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _PROVIDER_BACKBONE_PORT_H
#define _PROVIDER_BACKBONE_PORT_H
#include <spine/include/spine-label-fib.h>
#include <rte_rwlock.h>
struct pbp_private{
	rte_rwlock_t pbp_guard;
	struct spine_label_entry * label_base;
};
struct pbp_cache_entry{
	uint32_t                      is_valid;
	uint32_t 			 		  label;/*0 indicate invalid*/
	struct spine_label_entry 	* lentry;
	struct common_nexthop    	* unicast_nexthop;
	struct common_neighbor 		* unicast_neighbor;
	struct multicast_next_hops  * multicast_nexthops;
};
#define PBP_CACHE_SIZE 0x8
#define PBP_CACHE_MASK (PBP_CACHE_SIZE-1)

#define PBP_PROCESS_INPUT_DROP 0x0
#define PBP_PROCESS_INPUT_UNICAST_FWD 0x1
#define PBP_PROCESS_INPUT_MULTICAST_FWD 0x2
#define PBP_PROCESS_INPUT_HOST_STACK 0x3

#endif
