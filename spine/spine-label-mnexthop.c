/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <spine/include/spine-label-mnexthop.h>
#include <e3infra/include/e3-init.h>
#include <e3infra/include/e3-log.h>
#include <rte_malloc.h>
#include <string.h>
#include <stdlib.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/util.h>
#include <rte_rwlock.h>

static rte_rwlock_t mnexthop_guard;

struct multicast_next_hops  * mnext_hops_base;

#define RLOCK_MNEXTHOP()   rte_rwlock_read_lock(&mnexthop_guard)
#define RUNLOCK_MNEXTHOP() rte_rwlock_read_unlock(&mnexthop_guard)
#define WLOCK_MNEXTHOP()   rte_rwlock_write_lock(&mnexthop_guard)
#define WUNLOCK_MNEXTHOP() rte_rwlock_write_unlock(&mnexthop_guard)

void __read_lock_mnexthop(void)
{
	RLOCK_MNEXTHOP();
}

void __read_unlock_mnexthop(void)
{
	RUNLOCK_MNEXTHOP();
}

void spine_label_mnexthop_init(void)
{
	mnext_hops_base=rte_zmalloc(NULL,
			sizeof(struct multicast_next_hops)*MAX_MULTICAST_NEXT_HOPS,
			64);
	E3_ASSERT(mnext_hops_base);
	rte_rwlock_init(&mnexthop_guard);
}
E3_init(spine_label_mnexthop_init, TASK_PRIORITY_RESOURCE_INIT);



int register_multicast_nexthops()
{
	int ret=-E3_ERR_GENERIC;
	int idx=0;
	WLOCK_MNEXTHOP();
	for(idx=0;idx<MAX_MULTICAST_NEXT_HOPS;idx++)
		if(!mnext_hops_base[idx].is_valid)
			break;
	if(idx>=MAX_MULTICAST_NEXT_HOPS){
		ret=-E3_ERR_OUT_OF_RES;
		goto out;
	}
	memset(&mnext_hops_base[idx],0,sizeof(struct multicast_next_hops));
	mnext_hops_base[idx].index=idx;
	mnext_hops_base[idx].nr_hops=0;
	mnext_hops_base[idx].ref_cnt=0;
	mnext_hops_base[idx].is_valid=1;
	ret=idx;
	out:
	WUNLOCK_MNEXTHOP();
	return ret;
}


