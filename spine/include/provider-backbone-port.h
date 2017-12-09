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

#endif
