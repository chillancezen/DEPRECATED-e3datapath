#include <e3_init.h>
#include <e3-api-wrapper.h>
#include <e3iface-inventory.h>
#include <provider-backbone-port.h>

/*actually, it should be aliased to set_label_entry*/
e3_type register_label_entry(e3_type service,
							e3_type iface,/*input*/
							e3_type index_to_set,/*input*/
							e3_type entry/*input*/)
{
	uint16_t _iface=e3_type_to_uint16_t(iface);
	uint32_t _index_to_set=e3_type_to_uint32_t(index_to_set);
	struct label_entry * _entry=(struct label_entry*)
							e3_type_to_uint8_t_ptr(entry);

	struct E3Interface * piface=find_e3interface_by_index(_iface);
	struct pbp_private * priv=NULL;
	struct label_entry * label;
	if((!piface)||(piface->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT))
		return -1;
	if((_index_to_set<0)||(_index_to_set>=NR_LABEL_ENTRY))
		return -2;
	if(!(priv=(struct pbp_private*)piface->private))
		return -3;
	label=label_entry_at(priv->label_base,_index_to_set);
	label->is_unicast   =_entry->is_unicast;
	label->swapped_label=_entry->swapped_label;
	label->NHLFE        =_entry->NHLFE;
	label->is_valid=1;
	return 0;
}
DECLARE_E3_API(label_entry_registration)={
	.api_name="register_label_entry",
	.api_desc="register a label entry on a given E3 interface",
	.api_callback_func=(api_callback_func)register_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,
				.len=sizeof(struct label_entry)},
		{.type=e3_arg_type_none},
	},
};

e3_type get_label_entry(e3_type service,
						e3_type e3iface,/*input*/
						e3_type index,/*input*/
						e3_type entry/*output*/)
{
	uint16_t             _iface=e3_type_to_uint16_t(e3iface);
	uint32_t             _index=e3_type_to_uint32_t(index);
	struct label_entry * _entry=(struct label_entry*)
							e3_type_to_uint8_t_ptr(entry);
	struct E3Interface * pif =find_e3interface_by_index(_iface);
	struct pbp_private * priv=NULL;
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)||
		(!(priv=(struct pbp_private*)pif->private)))
		return -1;
	if((_index<0)||(_index>=NR_LABEL_ENTRY))
		return -2;
	
	memcpy(_entry,
			&priv->label_base[_index],
			sizeof(struct label_entry));
	return 0;
}

DECLARE_E3_API(label_entry_get)={
	.api_name="get_label_entry",
	.api_desc="get the label entry at a given index on a specific e3 iface",
	.api_callback_func=(api_callback_func)get_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(struct label_entry),},
		{.type=e3_arg_type_none},
	},
};
#define MAX_LABEL_ENTRIES_PER_FETCH 256
e3_type list_label_entry_partial(e3_type service,
									e3_type e3iface,/*input*/
									e3_type index_to_start,/*input&ouput*/
									e3_type nr_entries,/*input&ouput*/
									e3_type entries,/*input&ouput*/
									e3_type index_entries/*input&ouput*/)
{
	uint16_t _iface					=e3_type_to_uint16_t(e3iface);
	int * _index_to_start			=(int*)e3_type_to_uint8_t_ptr(index_to_start);
	int * _nr_entries				=(int*)e3_type_to_uint8_t_ptr(nr_entries);
	struct label_entry * _entries	=(struct label_entry *)
										e3_type_to_uint8_t_ptr(entries);
	uint32_t * _index_entries		=(uint32_t*)e3_type_to_uint8_t_ptr(index_entries);
	
	struct E3Interface * pif 		=find_e3interface_by_index(_iface);
	struct pbp_private * priv		=NULL;
	int idx=0;
	int iptr=0;
	
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)||
		(!(priv=(struct pbp_private*)pif->private)))
		return -1;
	for(idx=*_index_to_start;idx<NR_LABEL_ENTRY;idx++){
		if(!priv->label_base[idx].is_valid)
			continue;
		_index_entries[iptr]=idx;
		memcpy(&_entries[iptr++],
				&priv->label_base[idx],
				sizeof(struct label_entry));
		if(iptr==MAX_LABEL_ENTRIES_PER_FETCH)
			break;
	}
	*_index_to_start=idx+1;
	*_nr_entries=iptr;
	return 0;
}
DECLARE_E3_API(partial_list_label_entry)={
	.api_name="list_label_entry_partial",
	.api_desc="get the partial list of label entry",
	.api_callback_func=(api_callback_func)list_label_entry_partial,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(struct label_entry)*MAX_LABEL_ENTRIES_PER_FETCH},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(uint32_t)*MAX_LABEL_ENTRIES_PER_FETCH},
		{.type=e3_arg_type_none},
	},
};
e3_type delete_label_entry(e3_type service,e3_type e3iface,e3_type label_index)
{
	uint16_t  _e3iface	     =e3_type_to_uint16_t(e3iface);
	uint32_t  _label_index   =e3_type_to_uint32_t(label_index);
	struct E3Interface * pif =find_e3interface_by_index(_e3iface);
	struct pbp_private * priv=NULL;
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)||
		(!(priv=(struct pbp_private*)pif->private)))
		return -1;
	if((_label_index<0)||(_label_index>=NR_LABEL_ENTRY))
		return -2;
	priv->label_base[_label_index].is_valid=0;
	return 0;
}
DECLARE_E3_API(label_entry_deletion)={
	.api_name="delete_label_entry",
	.api_desc="invalidate a label entry at a given index",
	.api_callback_func=(api_callback_func)delete_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};