/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3infra/include/e3-init.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3net/include/e3iface-inventory.h>
#include <spine/include/provider-backbone-port.h>
#include <e3infra/include/util.h>
#include <rte_memcpy.h>
#include <e3infra/include/e3-log.h>

/*
*ZERO will be returned if successful,
*a negative is returned upon failue
*/
e3_type spine_api_pbp_setup_label_entry(e3_type service,
							e3_type iface,/*input*/
							e3_type index_to_set,/*input*/
							e3_type entry/*input*/)
{
	int ret=-E3_ERR_GENERIC;
	uint16_t _iface=e3_type_to_uint16_t(iface);
	uint32_t _index_to_set=e3_type_to_uint32_t(index_to_set);
	struct spine_label_entry * _entry=(struct spine_label_entry*)
							e3_type_to_uint8_t_ptr(entry);

	struct E3Interface * piface=find_e3interface_by_index(_iface);
	struct pbp_private * priv=NULL;
	if((!piface)||(piface->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT))
		return -E3_ERR_NOT_FOUND;
	if((_index_to_set<0)||(_index_to_set>=NR_SPINE_LABEL_ENTRY))
		return -E3_ERR_ILLEGAL;
	if(!(priv=(struct pbp_private*)piface->private))
		return -E3_ERR_ILLEGAL;
	rte_rwlock_write_lock(&priv->pbp_guard);
	ret=set_spine_label_entry(priv->label_base,
		_index_to_set,
		_entry->is_unicast,
		_entry->swapped_label,
		_entry->NHLFE);
	rte_rwlock_write_unlock(&priv->pbp_guard);
	E3_LOG("map pbport %d's label entry %d as %s with <nhlfe:%d,label:%d> and result as %d\n",
		_iface,
		_index_to_set,
		_entry->is_unicast?"unicast":"multicast",
		_entry->NHLFE,
		_entry->swapped_label,
		ret);
	return ret;
}
DECLARE_E3_API(spine_label_entry_registration)={
	.api_name="spine_api_pbp_setup_label_entry",
	.api_desc="register a label entry on a given E3 interface",
	.api_callback_func=(api_callback_func)spine_api_pbp_setup_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,
				.len=sizeof(struct spine_label_entry)},
		{.type=e3_arg_type_none},
	},
};

e3_type spine_api_pbp_get_label_entry(e3_type service,
						e3_type e3iface,/*input*/
						e3_type index,/*input*/
						e3_type entry/*output*/)
{
	uint16_t             _iface=e3_type_to_uint16_t(e3iface);
	uint32_t             _index=e3_type_to_uint32_t(index);
	struct spine_label_entry * _entry=(struct spine_label_entry*)e3_type_to_uint8_t_ptr(entry);
	
	struct E3Interface * pif =find_e3interface_by_index(_iface);
	struct pbp_private * priv=NULL;
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)||
		(!(priv=(struct pbp_private*)pif->private)))
		return -E3_ERR_NOT_FOUND;
	if((_index<0)||(_index>=NR_SPINE_LABEL_ENTRY))
		return -E3_ERR_ILLEGAL;
	rte_rwlock_read_lock(&priv->pbp_guard);
	rte_memcpy(_entry,
			&priv->label_base[_index],
			sizeof(struct spine_label_entry));
	rte_rwlock_read_unlock(&priv->pbp_guard);
	return E3_OK;
}

DECLARE_E3_API(spine_label_entry_get)={
	.api_name="spine_api_pbp_get_label_entry",
	.api_desc="get the label entry at a given index on a specific e3 iface",
	.api_callback_func=(api_callback_func)spine_api_pbp_get_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(struct spine_label_entry),},
		{.type=e3_arg_type_none},
	},
};
#define MAX_SPINE_LABEL_ENTRIES_PER_FETCH 256
e3_type cbp_api_list_spine_label_entry_partial(e3_type service,
									e3_type e3iface,/*input*/
									e3_type index_to_start,/*input&ouput*/
									e3_type nr_entries,/*input&ouput*/
									e3_type entries,/*input&ouput*/
									e3_type index_entries/*input&ouput*/)
{
	uint16_t _iface					=e3_type_to_uint16_t(e3iface);
	int * _index_to_start			=(int*)e3_type_to_uint8_t_ptr(index_to_start);
	int * _nr_entries				=(int*)e3_type_to_uint8_t_ptr(nr_entries);
	struct spine_label_entry * _entries	=(struct spine_label_entry *)
										e3_type_to_uint8_t_ptr(entries);
	uint32_t * _index_entries		=(uint32_t*)e3_type_to_uint8_t_ptr(index_entries);
	
	struct E3Interface * pif 		=find_e3interface_by_index(_iface);
	struct pbp_private * priv		=NULL;
	int idx=0;
	int iptr=0;
	
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)||
		(!(priv=(struct pbp_private*)pif->private)))
		return -E3_ERR_NOT_FOUND;
	rte_rwlock_read_lock(&priv->pbp_guard);
	for(idx=*_index_to_start;idx<NR_SPINE_LABEL_ENTRY;idx++){
		if(!priv->label_base[idx].is_valid)
			continue;
		_index_entries[iptr]=idx;
		rte_memcpy(&_entries[iptr++],
				&priv->label_base[idx],
				sizeof(struct spine_label_entry));
		if(iptr==MAX_SPINE_LABEL_ENTRIES_PER_FETCH)
			break;
	}
	rte_rwlock_read_unlock(&priv->pbp_guard);
	*_index_to_start=idx+1;
	*_nr_entries=iptr;
	return E3_OK;
}
DECLARE_E3_API(partial_list_spine_label_entry)={
	.api_name="cbp_api_list_spine_label_entry_partial",
	.api_desc="get the partial list of label entry",
	.api_callback_func=(api_callback_func)cbp_api_list_spine_label_entry_partial,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(struct spine_label_entry)*MAX_SPINE_LABEL_ENTRIES_PER_FETCH},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(uint32_t)*MAX_SPINE_LABEL_ENTRIES_PER_FETCH},
		{.type=e3_arg_type_none},
	},
};
e3_type spine_api_pbp_delete_label_entry(e3_type service,e3_type e3iface,e3_type label_index)
{
	int ret=-E3_ERR_GENERIC;
	uint16_t  _e3iface	     =e3_type_to_uint16_t(e3iface);
	uint32_t  _label_index   =e3_type_to_uint32_t(label_index);
	struct E3Interface * pif =find_e3interface_by_index(_e3iface);
	struct pbp_private * priv=NULL;
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)||
		(!(priv=(struct pbp_private*)pif->private)))
		return -E3_ERR_NOT_FOUND;
	if((_label_index<0)||(_label_index>=NR_SPINE_LABEL_ENTRY))
		return -E3_ERR_ILLEGAL;
	
	rte_rwlock_write_lock(&priv->pbp_guard);
	ret=reset_spine_label_entry(priv->label_base, _label_index);
	rte_rwlock_write_unlock(&priv->pbp_guard);
	E3_LOG("unmap pbport %d's label entry %d with result as %d\n",
		_e3iface,
		_label_index,
		ret);
	return ret;
}
DECLARE_E3_API(spine_label_entry_deletion)={
	.api_name="spine_api_pbp_delete_label_entry",
	.api_desc="invalidate a label entry at a given index",
	.api_callback_func=(api_callback_func)spine_api_pbp_delete_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};