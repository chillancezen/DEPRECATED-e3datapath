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
	uint8_t  is_valid:1;
	uint8_t  egress_nhlfe_index:7;
	/*
	*for outbound_nhlfe_index,
	*only meanfully in E-LAN,
	*all its bits with 1s is invalid,i.e. 0x7f.
	*this field eases finding path back to the source site
	*/
	uint8_t  e3_service;
	uint16_t service_index;
}__attribute__((packed));

/*
Python definition:
size of leaf_label_entry: 4
<Field type=c_ubyte, ofs=0:0, bits=1> is_valid
<Field type=c_ubyte, ofs=0:1, bits=7> egress_nhlfe_index
<Field type=c_ubyte, ofs=1, size=1> e3_service
<Field type=c_short, ofs=2, size=2> service_index
*/
struct leaf_label_entry * allocate_leaf_label_base(int numa_socket_id);

#define leaf_label_entry_at(base,index) ((((index)>=0)&&((index)<NR_LEAF_LABEL_ENTRY))?&(base)[(index)]:NULL)

int set_leaf_label_entry(struct leaf_label_entry * base,
	uint32_t index,
	struct leaf_label_entry *tmp_entry);

void reset_leaf_label_entry(struct leaf_label_entry *base,int index);
int set_leaf_label_entry_egress_nhlfe_index(struct leaf_label_entry *base,
	uint32_t label_index,
	uint32_t NHLFE,
	uint32_t label_to_push);
int clear_leaf_label_entry_egress_nhlfe_index(struct leaf_label_entry * base,
	uint32_t label_index);

#endif