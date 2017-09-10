#ifndef _LABEL_FIB_H
#define _LABEL_FIB_H
#include <stdint.h>

#define NR_LABEL_ENTRY (1<<20)

struct label_entry{
	union{
		struct{
			uint32_t is_valid:1;        /*indicate whether the corresponding ILE is valid*/
			uint32_t is_unicast:1;      /*if valid, whether the LSP is unicast(multicast)*/
			uint32_t swapped_label:20;  /*the label to swap*/
			uint32_t NHLFE:10;          /*next hop label forwarding entry*/
		};
		uint32_t dummy_dword;
	};
}__attribute__((packed));



struct label_entry * allocate_label_entry_base(int numa_socket_id);


/*retrieve the entry at the given index,if successful,
*the pointer will be returned.
*/
#define label_entry_at(base,index)  ((((index)>=0)&&((index)<NR_LABEL_ENTRY))?(&((base)[(index)])):NULL)

/*
*transform entry into index within a base,
*negative value returned if entry is not in the basis 
*/
#define label_entry_to_index(base,entry) ({\
	int _index=(struct label_entry*)(entry)-(struct label_entry*)(base); \
	((_index<NR_LABEL_ENTRY)&&(_index>=0))?_index:-1; \
})



/*populate the entry base for valiad entries,
*Input:base,next_index_to_search
*output:entry
*/
#define FOREACH_LABEL_ENTRY_INSIDE_BASE_START(base,next_index_to_search,entry) {\
	int _idx; \
	struct label_entry * _entry; \
	for(_idx=(next_index_to_search);_idx<NR_LABEL_ENTRY;_idx++){ \
		_entry=label_entry_at((base), _idx); \
		if(!_entry->is_valid) continue; \
		(entry)=_entry; 

#define FOREACH_LABEL_ENTRY_INSIDE_BASE_END() }}
	
int invalidate_label_entry(struct label_entry * base,int index);
int set_label_entry(struct label_entry * base,
					int index,
					int is_unicast,
					int label_to_swap,
					int NHLFE);

#endif