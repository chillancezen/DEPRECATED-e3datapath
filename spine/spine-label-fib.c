/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <spine/include/spine-label-fib.h>
#include <rte_malloc.h>
#include <e3net/include/common-nhlfe.h>
#include <spine/include/spine-label-mnexthop.h>
#include <e3infra/include/util.h>

/*
*allocate an array from DPDK memory allocator which is numa aware
*if numa_socket_id is negative, a random socket is selected,
*once succeed, a Non-NULL array pointer is returned. 
*/
struct spine_label_entry * allocate_label_entry_base(int numa_socket_id)
{
	int idx=0;
	struct spine_label_entry * base=NULL;
	base=rte_zmalloc_socket(NULL,
			sizeof(struct spine_label_entry)*NR_SPINE_LABEL_ENTRY,
			64,
			numa_socket_id<0?SOCKET_ID_ANY:numa_socket_id);
	if(base)
		for(;idx<NR_SPINE_LABEL_ENTRY;idx++)
			base[idx].is_valid=0;
	return base;
}

void update_spine_label_entry_relationship(struct spine_label_entry * base,
	int index,
	int is_to_remove)
{
	int idx=0;
	int is_existing;
	struct spine_label_entry * pentry=spine_label_entry_at(base, index);
	if(!pentry||!pentry->is_valid)
		return;
	is_existing=0;
	for(idx=0;idx<NR_SPINE_LABEL_ENTRY;idx++){
		if(!base[idx].is_valid)
			continue;
		if(index==idx)
			continue;
		if(base[idx].is_unicast!=pentry->is_unicast)
			continue;
		if(base[idx].NHLFE==pentry->NHLFE){
			is_existing=1;
			break;
		}
	}
	if(pentry->is_unicast){
		/*
		*found as unicast nexthop,manipulate the common_nexthop
		*/
		if(is_to_remove){
			if(!is_existing)
				dereference_common_nexthop(pentry->NHLFE);
		}else{
			if(!is_existing)
				reference_common_nexthop(pentry->NHLFE);
		}
	}else{
		/*
		*found as multicast nexthops
		*/
		if(is_to_remove){
			if(!is_existing)
				dereference_mnexthop(pentry->NHLFE);
		}else{
			if(!is_existing)
				reference_mnexthop(pentry->NHLFE);
		}
	}
}
int reset_spine_label_entry(struct spine_label_entry * base,int index)
{
	struct spine_label_entry * entry=spine_label_entry_at(base, index);
	if(!entry)
		return -E3_ERR_NOT_FOUND;
	update_spine_label_entry_relationship(base,
			index,
			1);
	entry->is_valid=0;
	entry->swapped_label=0;
	entry->NHLFE=0;
	return E3_OK;
}

/*note this is not multithread safe
*make sure a per-base guard lock is acuiqred before going on
*/
int set_spine_label_entry(struct spine_label_entry * base,
					int index,
					int is_unicast,
					int label_to_swap,
					int NHLFE)
{
	struct spine_label_entry * entry=spine_label_entry_at(base, index);
	if(!entry)
		return -E3_ERR_NOT_FOUND;
	if(entry->is_valid){
		update_spine_label_entry_relationship(base,
			index,
			1);
	}
	
	entry->is_unicast=!!is_unicast;
	entry->NHLFE=NHLFE;
	entry->swapped_label=label_to_swap;
	__sync_synchronize();
	entry->is_valid=1;
	update_spine_label_entry_relationship(base,index,0);
	return E3_OK;
}

void label_fib_module_test(void)
{
	#if 0
	//printf("size:%d\n",sizeof(struct label_entry));
	struct spine_label_entry * base=allocate_label_entry_base(-1);
	spine_label_entry_at(base, 1)->is_valid=1;
	spine_label_entry_at(base, 1023)->is_valid=1;
	spine_label_entry_at(base, 102223)->is_valid=1;

	set_label_entry(base,232,1,12,2);
	
	struct spine_label_entry * entry;
	
	//invalidate_label_entry(base,102223);
	
	FOREACH_LABEL_ENTRY_INSIDE_BASE_START(base,0,entry){
		printf("%d\n",spine_label_entry_to_index(base, entry));
	}
	FOREACH_LABEL_ENTRY_INSIDE_BASE_END()
	
	
	//printf("%d\n",sizeof(struct label_entry));
	#endif
}
