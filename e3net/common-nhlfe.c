/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3net/include/common-nhlfe.h>
#include <rte_malloc.h>
#include <e3_init.h>
#include <e3_log.h>
#include <rte_memcpy.h>
struct common_neighbor * neighbor_base=NULL;
struct common_nexthop  * nexthop_base=NULL;
void init_common_nhlfe(void)
{
	neighbor_base=rte_zmalloc(NULL,
			sizeof(struct common_neighbor)*MAX_COMMON_NEIGHBORS,
			64);
	E3_ASSERT(neighbor_base);
	nexthop_base=rte_zmalloc(NULL,
			sizeof(struct common_nexthop)*MAX_COMMON_NEXTHOPS,
			64);
	E3_ASSERT(nexthop_base);
}
E3_init(init_common_nhlfe,TASK_PRIORITY_RESOURCE_INIT);

/*
*return the actual index of registered neighbour
*upon any failure, a negative value is returned
*/
int register_common_neighbor(struct common_neighbor * neighbor)
{
	int idx=0;
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++){
		if(!neighbor_base[idx].is_valid)
			continue;
		if(neighbor_base[idx].neighbour_ip_as_le==neighbor->neighbour_ip_as_le)
			return -1;
	}
	for(idx=0;idx<MAX_COMMON_NEIGHBORS;idx++)
		if(!neighbor_base[idx].is_valid)
			break;
	if(idx>=MAX_COMMON_NEIGHBORS)
		return -2;
	neighbor_base[idx].index=idx;
	neighbor_base[idx].neighbour_ip_as_le=neighbor->neighbour_ip_as_le;
	neighbor_base[idx].ref_cnt=0;
	neighbor_base[idx].is_valid=1;
	rte_memcpy(neighbor_base[idx].mac,
		neighbor->mac,
		6);
	return idx;
}

int reference_common_nrighbor(int index)
{
	struct common_neighbor * neighbor=find_common_neighbor(index);
	if(!neighbor||!neighbor->is_valid)
		return -1;
	neighbor->ref_cnt++;
	return 0;
}
int dereference_common_neighbor(int index)
{
	struct common_neighbor * neighbor=find_common_neighbor(index);
	if(!neighbor||!neighbor->is_valid)
		return -1;
	if(neighbor->ref_cnt>0)
		neighbor->ref_cnt--;
	return 0;
}

int delete_common_neighbor(int index)
{
	struct common_neighbor * neighbor=find_common_neighbor(index);
	if(!neighbor||!neighbor->is_valid)
		return -1;
	if(neighbor->ref_cnt)
		return -2;
	neighbor->is_valid=0;
	return 0;
}
/*
*as with other registration entry,
*return the actual index,negative integer is returned 
*upon failure
*/
int register_common_nexthop(struct common_nexthop * hop)
{
	struct common_neighbor * neighbor;
	int idx=0;
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++){
		if(!nexthop_base[idx].is_valid)
			continue;
		/*
		*here we check whether <ifce,neighbor> conflicts
		*/
		if((nexthop_base[idx].local_e3iface==hop->local_e3iface)&&
			(nexthop_base[idx].common_neighbor_index==hop->common_neighbor_index))
			return -1;
	}
	/*
	*make sure the target neighbor exists
	*/
	neighbor=find_common_neighbor(hop->common_neighbor_index);
	if(!neighbor||!neighbor->is_valid)
		return -2;

	/*
	*todo:after adding ref count for e3 interface,
	*check whether local_e3iface field is legal
	*/
	
	for(idx=0;idx<MAX_COMMON_NEXTHOPS;idx++)
		if(!nexthop_base[idx].is_valid)
			break;
	if(idx>=MAX_COMMON_NEXTHOPS)
		return -3;
	nexthop_base[idx].index=idx;
	nexthop_base[idx].common_neighbor_index=hop->common_neighbor_index;
	nexthop_base[idx].local_e3iface=hop->local_e3iface;
	nexthop_base[idx].ref_cnt=0;
	nexthop_base[idx].is_valid=1;
	nexthop_base[idx].reserved0=0;
	nexthop_base[idx].reserved1=0;
	E3_ASSERT(!reference_common_nrighbor(neighbor->index));
	
	return idx;
}

int reference_common_nexthop(int index)
{
	struct common_nexthop * hop=find_common_nexthop(index);
	if(!hop||!hop->is_valid)
		return -1;
	hop->ref_cnt++;
	return 0;
}
int dereference_common_nexthop(int index)
{
	struct common_nexthop * hop=find_common_nexthop(index);
		if(!hop||!hop->is_valid)
			return -1;
	if(hop->ref_cnt>0)
		hop->ref_cnt--;
	return 0;
}
int delete_common_nexthop(int index)
{
	struct common_nexthop * hop=find_common_nexthop(index);
	if(!hop||!hop->is_valid)
		return -1;
	if(hop->ref_cnt)
		return -2;
	dereference_common_neighbor(hop->common_neighbor_index);
	hop->is_valid=0;
	return 0;
}