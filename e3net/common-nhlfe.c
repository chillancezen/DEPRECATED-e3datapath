/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3net/include/common-nhlfe.h>
#include <rte_malloc.h>
#include <e3infra/include/e3-init.h>
#include <e3infra/include/e3-log.h>
#include <rte_memcpy.h>
#include <rte_rwlock.h>
#include <e3infra/include/util.h>
#include <e3infra/include/malloc-wrapper.h>

struct common_neighbor * neighbor_base=NULL;
struct common_nexthop  * nexthop_base=NULL;
static rte_rwlock_t nexthop_guard;
static rte_rwlock_t neighbor_guard;

#define RLOCK_NEXTHOP() 	rte_rwlock_read_lock(&nexthop_guard)
#define WLOCK_NEXTHOP() 	rte_rwlock_write_lock(&nexthop_guard)
#define RUNLOCK_NEXTHOP()   rte_rwlock_read_unlock(&nexthop_guard)
#define WUNLOCK_NEXTHOP()   rte_rwlock_write_unlock(&nexthop_guard)
#define RLOCK_NEIGHBOR() 	rte_rwlock_read_lock(&neighbor_guard)
#define WLOCK_NEIGHBOR() 	rte_rwlock_write_lock(&neighbor_guard)
#define RUNLOCK_NEIGHBOR() 	rte_rwlock_read_unlock(&neighbor_guard)
#define WUNLOCK_NEIGHBOR() 	rte_rwlock_write_unlock(&neighbor_guard)

void __read_lock_neighbor(void)
{
	RLOCK_NEIGHBOR();
}
void __read_unlock_neighbor(void)
{
	RUNLOCK_NEIGHBOR();
}
void __read_lock_nexthop(void)
{
	RLOCK_NEXTHOP();
}
void __read_unlock_nexthop(void)
{
	RUNLOCK_NEXTHOP();
}
void init_common_nhlfe(void)
{
	neighbor_base=RTE_ZMALLOC(NULL,
			sizeof(struct common_neighbor)*MAX_COMMON_NEIGHBORS,
			64);
	E3_ASSERT(neighbor_base);
	nexthop_base=RTE_ZMALLOC(NULL,
			sizeof(struct common_nexthop)*MAX_COMMON_NEXTHOPS,
			64);
	E3_ASSERT(nexthop_base);
    rte_rwlock_init(&nexthop_guard);
    rte_rwlock_init(&neighbor_guard);
}
E3_init(init_common_nhlfe,TASK_PRIORITY_RESOURCE_INIT);

/*
*return the actual index of registered neighbour
*upon any failure, a negative value is returned
*/
int register_common_neighbor(struct common_neighbor * neighbor)
{
    int ret=-E3_ERR_GENERIC;
	int idx=0;
    WLOCK_NEIGHBOR();
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++){
		if(!neighbor_base[idx].is_valid)
			continue;
		if(neighbor_base[idx].neighbour_ip_as_le==neighbor->neighbour_ip_as_le){
            ret=-E3_ERR_ILLEGAL;
			goto out;
        }
	}
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		if(!neighbor_base[idx].is_valid)
			break;
	if(idx>=MAX_COMMON_NEIGHBORS){
        ret=-E3_ERR_OUT_OF_RES;
		goto out;
    }
	neighbor_base[idx].index=idx;
	neighbor_base[idx].neighbour_ip_as_le=neighbor->neighbour_ip_as_le;
	neighbor_base[idx].ref_cnt=0;
	neighbor_base[idx].is_valid=1;
	rte_memcpy(neighbor_base[idx].mac,
		neighbor->mac,
		6);
    ret=idx;
    out:
    WUNLOCK_NEIGHBOR();
	return ret;
}
int refresh_common_neighbor_mac(struct common_neighbor * neighbor)
{
	int ret=-E3_ERR_GENERIC;
	int idx=0;
	WLOCK_NEIGHBOR();
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++){
		if(!neighbor_base[idx].is_valid)
			continue;
		if(neighbor_base[idx].neighbour_ip_as_le==neighbor->neighbour_ip_as_le)
			break;
	}
	if(idx>=MAX_COMMON_NEIGHBORS){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	rte_memcpy(neighbor_base[idx].mac,
		neighbor->mac,
		6);
	ret=E3_OK;
	out:
	WUNLOCK_NEIGHBOR();
	return ret;
}

int reference_common_nrighbor(int index)
{
    int ret=-E3_ERR_GENERIC;
	struct common_neighbor * neighbor;
    WLOCK_NEIGHBOR();
    neighbor=find_common_neighbor(index);
	if(!neighbor)
		goto out;
	neighbor->ref_cnt++;
    ret=E3_OK;
    out:
    WUNLOCK_NEIGHBOR();
	return ret;
}
int dereference_common_neighbor(int index)
{
    int ret=-E3_ERR_GENERIC;
	struct common_neighbor * neighbor;
    WLOCK_NEIGHBOR();
    neighbor=find_common_neighbor(index);
	if(!neighbor)
		goto out;
	if(neighbor->ref_cnt>0)
		neighbor->ref_cnt--;
    ret=E3_OK;
    out:
    WUNLOCK_NEIGHBOR();
	return ret;
}

int delete_common_neighbor(int index)
{
    int ret=-E3_ERR_GENERIC;
	struct common_neighbor * neighbor;
    WLOCK_NEIGHBOR();
    neighbor=find_common_neighbor(index);
	if(!neighbor)
		goto out;
	if(neighbor->ref_cnt){
        ret=-E3_ERR_IN_USE;
        goto out;
    }
	neighbor->is_valid=0;
    ret=E3_OK;
    out:
    WUNLOCK_NEIGHBOR();
	return ret;
}

/*
*as with other registration entry,
*return the actual index,negative integer is returned 
*upon failure
*/
int register_common_nexthop(struct common_nexthop * hop)
{   
    int ret=-E3_ERR_GENERIC;
	struct common_neighbor * neighbor;
	int idx=0;
    WLOCK_NEXTHOP();
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++){
		if(!nexthop_base[idx].is_valid)
			continue;
		/*
		*here we check whether <ifce,neighbor> conflicts
		*/
		if((nexthop_base[idx].local_e3iface==hop->local_e3iface)&&
			(nexthop_base[idx].common_neighbor_index==hop->common_neighbor_index)){
			ret=-E3_ERR_ILLEGAL;
			goto out;
        }
	}
	/*
	*make sure the target neighbor exists
	*/
	__read_lock_neighbor();
	neighbor=find_common_neighbor(hop->common_neighbor_index);
	__read_unlock_neighbor();
	if(!neighbor){
        ret=-E3_ERR_NOT_FOUND;
		goto out;
    }

	/*
	*todo:after adding ref count for e3 interface,
	*check whether local_e3iface field is legal
	*/
	
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		if(!nexthop_base[idx].is_valid)
			break;
	if(idx>=MAX_COMMON_NEXTHOPS){
        ret=-E3_ERR_OUT_OF_RES;
		goto out;
    }
	nexthop_base[idx].index=idx;
	nexthop_base[idx].common_neighbor_index=hop->common_neighbor_index;
	nexthop_base[idx].local_e3iface=hop->local_e3iface;
	nexthop_base[idx].ref_cnt=0;
	nexthop_base[idx].is_valid=1;
	nexthop_base[idx].reserved0=0;
	nexthop_base[idx].reserved1=0;
	E3_ASSERT(!reference_common_nrighbor(neighbor->index));
    ret=idx;
	out:
    WUNLOCK_NEXTHOP();
	return ret;
}

int reference_common_nexthop(int index)
{
    int ret=-E3_ERR_GENERIC;
	struct common_nexthop * hop;
    WLOCK_NEXTHOP();
    hop=find_common_nexthop(index);
	if(!hop)
		goto out;
	hop->ref_cnt++;
    ret=E3_OK;
    out:
    WUNLOCK_NEXTHOP();
	return ret;
}
int dereference_common_nexthop(int index)
{
    int ret=-E3_ERR_GENERIC;
	struct common_nexthop * hop;
    WLOCK_NEXTHOP();
    hop=find_common_nexthop(index);
	if(!hop)
		goto out;
	if(hop->ref_cnt>0)
		hop->ref_cnt--;
    ret=E3_OK;
    out:
    WUNLOCK_NEXTHOP();
	return ret;
}
int delete_common_nexthop(int index)
{
    int ret=-E3_ERR_GENERIC;
	struct common_nexthop * hop;
    WLOCK_NEXTHOP();
    hop=find_common_nexthop(index);
	if(!hop){
        ret=-E3_ERR_NOT_FOUND;
		goto out;
    }
	if(hop->ref_cnt){
        ret=-E3_ERR_IN_USE;
		goto out;
    }
	dereference_common_neighbor(hop->common_neighbor_index);
	hop->is_valid=0;
    ret=E3_OK;
    out:
    WUNLOCK_NEXTHOP();
	return ret;
}
