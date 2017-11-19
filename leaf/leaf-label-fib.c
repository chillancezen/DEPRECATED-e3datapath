/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/leaf-label-fib.h>
#include <leaf/include/leaf-e-service.h>
#include <rte_malloc.h>
#include <e3infra/include/e3-log.h>
#include <e3infra/include/util.h>
struct leaf_label_entry * allocate_leaf_label_base(int numa_socket_id)
{
	struct leaf_label_entry * base;
	int idx=0;
	base=rte_zmalloc_socket(NULL,
			sizeof(struct leaf_label_entry)*NR_LEAF_LABEL_ENTRY,
			64,
			numa_socket_id<0?SOCKET_ID_ANY:numa_socket_id);
	for(;idx<NR_LEAF_LABEL_ENTRY;idx++)
		base[idx].is_valid=0;
	return base;
}

static
void update_leaf_label_entry_relationship(struct leaf_label_entry * base,
				int index,
				int is_removed)
{
	int idx=0;
	int is_existing;
	struct leaf_label_entry * pentry=leaf_label_entry_at(base,index);
	if(!pentry||!pentry->is_valid)
		return;
	switch(pentry->e3_service)
	{
		case e_line_service:
			for(idx=0,is_existing=0;idx<NR_LEAF_LABEL_ENTRY;idx++){
				if(!base[idx].is_valid)
					continue;
				if(idx==index)
					continue;
				if(base[idx].e3_service!=e_line_service)
					continue;
				if(base[idx].service_index==pentry->service_index){
					is_existing=1;
					break;
				}
			}
			if(is_removed){
				if(!is_existing)
					dereference_e_line_service(pentry->service_index);
			}else{
				if(!is_existing)
					reference_e_line_service(pentry->service_index);
			}
			break;
		case e_lan_service:
			for(idx=0,is_existing=0;idx<NR_LEAF_LABEL_ENTRY;idx++){
				if(!base[idx].is_valid)
					continue;
				if(idx==index)
					continue;
				if(base[idx].e3_service!=e_lan_service)
					continue;
				if(base[idx].service_index==pentry->service_index){
					is_existing=1;
					break;
				}
			}
			if(is_removed){
				if(!is_existing)
					dereference_e_lan_service(pentry->service_index);
			}else{
				if(!is_existing)
					reference_e_lan_service(pentry->service_index);
			}
			break;
		default:
			E3_ASSERT(0);/*"never expect the value"*/
			break;
	}
}


int set_leaf_label_entry(struct leaf_label_entry * base,
	uint32_t index,
	struct leaf_label_entry *tmp_entry)
{
	struct leaf_label_entry * pentry=leaf_label_entry_at(base,index);
	struct ether_e_line * eline;
	struct ether_e_lan  * elan;
	/*
	*check whether ether service is leagal
	*/
	if(!pentry)
		return -E3_ERR_GENERIC;
	switch(tmp_entry->e3_service)
	{
		case e_line_service:
			eline=find_e_line_service(tmp_entry->service_index);
			if(!eline||!eline->is_valid)
				return -E3_ERR_ILLEGAL;
			break;
		case e_lan_service:
			elan=find_e_lan_service(tmp_entry->service_index);
			if(!elan||!elan->is_valid)
				return -E3_ERR_ILLEGAL;
			break;
		default:
			return -E3_ERR_GENERIC;
			break;
	}
	/*
	*cleanup previous label entry relationship
	*/
	if(pentry->is_valid){
		update_leaf_label_entry_relationship(base,
			index,
			1);
	}
	base[index].e3_service=tmp_entry->e3_service;
	base[index].service_index=tmp_entry->service_index;
	base[index].is_valid=1;
	update_leaf_label_entry_relationship(base,
		index,
		0);
	return E3_OK;
}
void reset_leaf_label_entry(struct leaf_label_entry *base,int index)
{
	struct leaf_label_entry * pentry;
	update_leaf_label_entry_relationship(base,
		index,
		1);
	pentry=leaf_label_entry_at(base,index);
	if(pentry)
		pentry->is_valid=0;
}