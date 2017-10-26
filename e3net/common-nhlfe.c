#include <e3net/include/common-nhlfe.h>
#include <rte_malloc.h>
#include <e3_init.h>
#include <e3_log.h>
#include <rte_memcpy.h>
struct common_neighbor * neighbor_base=NULL;

void init_common_nhlfe(void)
{
	neighbor_base=rte_zmalloc(NULL,
			sizeof(struct common_neighbor)*MAX_COMMON_NEIGHBORS,
			64);
	E3_ASSERT(neighbor_base);
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
