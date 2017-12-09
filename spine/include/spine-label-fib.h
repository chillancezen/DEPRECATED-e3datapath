/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _LABEL_FIB_H
#define _LABEL_FIB_H
#include <stdint.h>

#define NR_LABEL_ENTRY (1<<20)
/*
*as with leaf_label_entry, spine_label_entry
*is guarded by per-pbp rwlock
*/
struct spine_label_entry{
	union{
		struct{
			uint32_t is_valid:1;        /*indicate whether the corresponding ILE is valid*/
			uint32_t is_unicast:1;      /*if valid, whether the LSP is unicast(multicast)*/
			uint32_t swapped_label:20;  /*the label to swap*/
										/*also as uint32_t RPF_CHECK_LABEL:20;
										*in multicast forwarding,perform rpf checking to
										*avoid duplicated replications
										*/
			uint32_t reserved:10;
			uint32_t NHLFE;          /*next hop label forwarding entry,
									 *expand to 32bit wide to accomondate enough (multicast
									 *netx hop entries)
									 */
		};
		uint64_t dummy_dword;
	};
}__attribute__((packed));
/*
C definition:

Python Definition:
<Field type=c_uint, ofs=0:0, bits=1>
<Field type=c_uint, ofs=0:1, bits=1>
<Field type=c_uint, ofs=0:2, bits=20>
<Field type=c_uint, ofs=0:22, bits=10>
<Field type=c_uint, ofs=4, size=4>
*/


struct spine_label_entry * allocate_label_entry_base(int numa_socket_id);


/*retrieve the entry at the given index,if successful,
*the pointer will be returned.
*/
#define spine_label_entry_at(base,index)  ((((index)>=0)&&((index)<NR_LABEL_ENTRY))?(&((base)[(index)])):NULL)

/*
*transform entry into index within a base,
*negative value returned if entry is not in the basis 
*/
#define spine_label_entry_to_index(base,entry) ({\
	int _index=(struct spine_label_entry*)(entry)-(struct spine_label_entry*)(base); \
	((_index<NR_LABEL_ENTRY)&&(_index>=0))?_index:-1; \
})



/*populate the entry base for valiad entries,
*Input:base,next_index_to_search
*output:entry
*/
#define FOREACH_LABEL_ENTRY_INSIDE_BASE_START(base,next_index_to_search,entry) {\
	int _idx; \
	struct spine_label_entry * _entry; \
	for(_idx=(next_index_to_search);_idx<NR_LABEL_ENTRY;_idx++){ \
		_entry=spine_label_entry_at((base), _idx); \
		if(!_entry->is_valid) continue; \
		(entry)=_entry; 

#define FOREACH_LABEL_ENTRY_INSIDE_BASE_END() }}
	
int invalidate_label_entry(struct spine_label_entry * base,int index);
int set_label_entry(struct spine_label_entry * base,
					int index,
					int is_unicast,
					int label_to_swap,
					int NHLFE);

#endif
