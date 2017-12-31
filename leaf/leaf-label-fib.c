/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/leaf-label-fib.h>
#include <leaf/include/leaf-e-service.h>
#include <rte_malloc.h>
#include <e3infra/include/e3-log.h>
#include <e3infra/include/util.h>
#include <e3infra/include/malloc-wrapper.h>
struct leaf_label_entry * allocate_leaf_label_base(int numa_socket_id)
{
	struct leaf_label_entry * base;
	int idx=0;
	base=RTE_ZMALLOC_SOCKET(NULL,
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
					dereference_e_line_service_locked(pentry->service_index);
			}else{
				if(!is_existing)
					reference_e_line_service_locked(pentry->service_index);
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
					dereference_e_lan_service_locked(pentry->service_index);
			}else{
				if(!is_existing)
					reference_e_lan_service_locked(pentry->service_index);
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
			tmp_entry->egress_nhlfe_index=0x7f;
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
	base[index].egress_nhlfe_index=tmp_entry->egress_nhlfe_index;
	__sync_synchronize();
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
int set_leaf_label_entry_egress_nhlfe_index(struct leaf_label_entry *base,
	uint32_t label_index,
	uint32_t NHLFE,
	uint32_t label_to_push)
{
	struct ether_e_lan *elan;
	struct leaf_label_entry *llentry=NULL;
	int egress_nhlfe_index=0x7f;

	if((!(llentry=leaf_label_entry_at(base,label_index)))||
		(!llentry->is_valid)||
		(llentry->e3_service!=e_lan_service)||
		(!(elan=find_e_lan_service(llentry->service_index))))
		return -E3_ERR_ILLEGAL;
	/*
	*make sure <NHLFE, label_to_push> is an nhlfe in e-lan's nhlfes array
	*the will acquire E_LAN's global Read Lock
	*/
	if((egress_nhlfe_index=find_e_lan_nhlfe_locked(elan->index,NHLFE,label_to_push))<0)
		return -E3_ERR_ILLEGAL;
	llentry->egress_nhlfe_index=egress_nhlfe_index;
	return E3_OK;
}

int clear_leaf_label_entry_egress_nhlfe_index(struct leaf_label_entry * base,
	uint32_t label_index)
{
	struct leaf_label_entry * llentry=leaf_label_entry_at(base,label_index);
	if(!llentry||
		!llentry->is_valid)
		return -E3_ERR_ILLEGAL;
	llentry->egress_nhlfe_index=0x7f;
	return E3_OK;
}