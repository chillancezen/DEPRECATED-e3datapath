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
#include <e3net/include/common-nhlfe.h>
#include <e3infra/include/malloc-wrapper.h>

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
	mnext_hops_base=RTE_ZMALLOC(NULL,
			sizeof(struct multicast_next_hops)*MAX_MULTICAST_NEXT_HOPS,
			64);
	E3_ASSERT(mnext_hops_base);
	rte_rwlock_init(&mnexthop_guard);
}
E3_init(spine_label_mnexthop_init, TASK_PRIORITY_RESOURCE_INIT);



int register_multicast_nexthop()
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
	E3_LOG("register multicast nexthops with result as %d\n",ret);
	return ret;
}

int delete_multicast_nexthop(int mindex)
{
	int idx=0;
	int ret=-E3_ERR_GENERIC;
	struct multicast_next_hops * pmnexthop;
	WLOCK_MNEXTHOP();
	pmnexthop=find_mnext_hops(mindex);
	if(!pmnexthop){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	if(pmnexthop->ref_cnt){
		ret=-E3_ERR_IN_USE;
		goto out;
	}
	/*
	*ready to dereference the genuine nexthops
	*/
	for(idx=0;idx<MAX_HOPS_IN_MULTICAST_GROUP;idx++){
		if(!pmnexthop->nexthops[idx].is_valid)
			continue;
		dereference_common_nexthop(pmnexthop->nexthops[idx].next_hop);
		pmnexthop->nexthops[idx].is_valid=0;
	}
	pmnexthop->nr_hops=0;
	pmnexthop->is_valid=0;
	ret=E3_OK;
	out:
	WUNLOCK_MNEXTHOP();
	E3_LOG("delete multicast nexthop entry %d with result as %d\n",
		mindex,
		ret);
	return ret;
}

int register_nexthop_in_mnexthops(int16_t mnexhop,int16_t nexthop,uint32_t label_to_push)
{
	int idx=0;
	int ret=-E3_ERR_GENERIC;
	struct multicast_next_hops * pmnexthop;
	WLOCK_MNEXTHOP();
	pmnexthop=find_mnext_hops(mnexhop);
	if(!pmnexthop){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	/*
	*duplication sanity check
	*/
	for(idx=0;idx<MAX_HOPS_IN_MULTICAST_GROUP;idx++){
		if(!pmnexthop->nexthops[idx].is_valid)
			continue;
		if((pmnexthop->nexthops[idx].next_hop==nexthop)&&
			(pmnexthop->nexthops[idx].label_to_push==label_to_push))
			break;
	}
	if(idx<MAX_HOPS_IN_MULTICAST_GROUP){
		ret=-E3_ERR_ILLEGAL;
		goto out;
	}
	/*
	*find a slot for newly joint nexthop entry
	*/
	for(idx=0;idx<MAX_HOPS_IN_MULTICAST_GROUP;idx++){
		if(!pmnexthop->nexthops[idx].is_valid)
			break;
	}
	if(idx>=MAX_HOPS_IN_MULTICAST_GROUP){
		ret=-E3_ERR_OUT_OF_RES;
		goto out;
	}
	/*
	*reference nexthop fisrt
	*/
	if(reference_common_nexthop(nexthop)){
		ret=-E3_ERR_ILLEGAL;
		goto out;
	}
	pmnexthop->nexthops[idx].next_hop=nexthop;
	pmnexthop->nexthops[idx].label_to_push=label_to_push;
	pmnexthop->nexthops[idx].is_valid=1;
	pmnexthop->nr_hops++;
	E3_ASSERT(pmnexthop->nr_hops<=MAX_HOPS_IN_MULTICAST_GROUP);
	ret=idx;
	out:
	WUNLOCK_MNEXTHOP();
	E3_LOG("register <nexthop:%d,label:%d> in multicast nexthops entry %d with result as %d\n",
		nexthop,
		label_to_push,
		mnexhop,
		ret);
	return ret;
}

int delete_nexthop_in_mnexthops(int16_t mnexhop,int16_t nexthop,uint32_t label_to_push)
{
	int idx=0;
	int ret=-E3_ERR_GENERIC;
	int iptr=0;
	struct multicast_next_hops * pmnexthop;
	WLOCK_MNEXTHOP();
	pmnexthop=find_mnext_hops(mnexhop);
	if(!pmnexthop){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	/*
	*try to find the inner nexthop index inside
	*/
	for(iptr=-1,idx=0;idx<MAX_HOPS_IN_MULTICAST_GROUP;idx++){
		if(!pmnexthop->nexthops[idx].is_valid)
			continue;
		if((pmnexthop->nexthops[idx].next_hop==nexthop)&&
			(pmnexthop->nexthops[idx].label_to_push==label_to_push)){
			iptr=idx;
			break;
		}
	}
	if(iptr<0){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	/*
	*once found, derefece it
	*/
	dereference_common_nexthop(pmnexthop->nexthops[idx].next_hop);
	pmnexthop->nexthops[idx].is_valid=0;
	pmnexthop->nr_hops--;
	E3_ASSERT(pmnexthop->nr_hops>=0);
	ret=E3_OK;
	out:
	WUNLOCK_MNEXTHOP();
	E3_LOG("delete <nexthop:%d,label:%d> in multicast nexthops entry %d with result as %d\n",
		nexthop,
		label_to_push,
		mnexhop,
		ret);
	return ret;
}

int reference_mnexthop(int mnexthop)
{

	int ret=-E3_ERR_GENERIC;
	struct multicast_next_hops * pmnexthop;
	WLOCK_MNEXTHOP();
	pmnexthop=find_mnext_hops(mnexthop);
	if(!pmnexthop){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	pmnexthop->ref_cnt++;
	ret=E3_OK;
	out:
	WUNLOCK_MNEXTHOP();
	return ret;
}

int dereference_mnexthop(int mnexthop)
{
	
	int ret=-E3_ERR_GENERIC;
	struct multicast_next_hops * pmnexthop;
	WLOCK_MNEXTHOP();
	pmnexthop=find_mnext_hops(mnexthop);
	if(!pmnexthop){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	if(pmnexthop->ref_cnt>0)
		pmnexthop->ref_cnt--;
	ret=E3_OK;
	out:
	WUNLOCK_MNEXTHOP();
	return ret;
}

