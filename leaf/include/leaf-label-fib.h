/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _LEAF_LABEL_FIB_H
#define _LEAF_LABEL_FIB_H
#include<stdint.h>
#include <e3net/include/common-nhlfe.h>

enum e_service{
	e_line_service=0,
	e_lan_service=1,
};
#define NR_LEAF_LABEL_ENTRY (1<<20)

struct leaf_label_entry{
	uint8_t  is_valid;
	uint8_t  e3_service;
	uint16_t service_index;
}__attribute__((packed));

struct leaf_label_entry * allocate_leaf_label_base(int numa_socket_id);

#define leaf_label_entry_at(base,index) ((((index)>=0)&&((index)<NR_LEAF_LABEL_ENTRY))?&(base)[(index)]:NULL)

int set_leaf_label_entry(struct leaf_label_entry * base,
	uint32_t index,
	struct leaf_label_entry *tmp_entry);

void reset_leaf_label_entry(struct leaf_label_entry *base,int index);

#endif