/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _CUSTOMER_BACKBONE_PORT_H
#define _CUSTOMER_BACKBONE_PORT_H
#include <leaf/include/leaf-label-fib.h>
#include <rte_rwlock.h>
struct cbp_private{
	rte_rwlock_t cbp_guard;
	struct leaf_label_entry *label_base;
};
#define CBP_NODE_BURST_SIZE 48

#define CBP_CACHE_SIZE 8
#define CBP_CACHE_MASK (CBP_CACHE_SIZE-1)

#define MAC_CACHE_SIZE 8
#define MAC_CACHE_MASK (MAC_CACHE_SIZE-1)

#define CBP_PROCESS_INPUT_DROP 0x0
#define CBP_PROCESS_INPUT_ELINE_FWD 0x1
#define CBP_PROCESS_INPUT_ELAN_UNICAST_FWD 0x2
#define CBP_PROCESS_INPUT_ELAN_MULTICAST_FWD 0x3
#define CBP_PROCESS_INPUT_HOST_STACK 0x4
#endif