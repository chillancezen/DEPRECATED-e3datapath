/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <spine-label-fib.h>
#include <rte_malloc.h>


/*
*allocate an array from DPDK memory allocator which is numa aware
*if numa_socket_id is negative, a random socket is selected,
*once succeed, a Non-NULL array pointer is returned. 
*/
struct label_entry * allocate_label_entry_base(int numa_socket_id)
{
	int idx=0;
	struct label_entry * base=NULL;
	base=rte_zmalloc_socket(NULL,
			sizeof(struct label_entry)*NR_LABEL_ENTRY,
			64,
			numa_socket_id<0?SOCKET_ID_ANY:numa_socket_id);
	if(base)
		for(;idx<NR_LABEL_ENTRY;idx++)
			base[idx].is_valid=0;
	return base;
}

int invalidate_label_entry(struct label_entry * base,int index)
{
	struct label_entry * entry=label_entry_at(base, index);
	if(!entry)
		return -1;
	entry->is_valid=0;
	entry->swapped_label=0;
	entry->NHLFE=0;
	return 0;
}

int set_label_entry(struct label_entry * base,
					int index,
					int is_unicast,
					int label_to_swap,
					int NHLFE)
{
	struct label_entry * entry=label_entry_at(base, index);
	if(!entry)
		return -1;
	entry->is_valid=1;
	entry->is_unicast=!!is_unicast;
	entry->NHLFE=NHLFE;
	entry->swapped_label=label_to_swap;
	return 0;
}

void label_fib_module_test(void)
{
	//printf("size:%d\n",sizeof(struct label_entry));
	struct label_entry * base=allocate_label_entry_base(-1);
	label_entry_at(base, 1)->is_valid=1;
	label_entry_at(base, 1023)->is_valid=1;
	label_entry_at(base, 102223)->is_valid=1;

	set_label_entry(base,232,1,12,2);
	
	struct label_entry * entry;
	
	//invalidate_label_entry(base,102223);
	
	FOREACH_LABEL_ENTRY_INSIDE_BASE_START(base,0,entry){
		printf("%d\n",label_entry_to_index(base, entry));
	}
	FOREACH_LABEL_ENTRY_INSIDE_BASE_END()
	
	
	//printf("%d\n",sizeof(struct label_entry));
}
