#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/util.h>
#include <rte_memcpy.h>
#include <spine/include/spine-label-mnexthop.h>

/*
*retutrn the actual mnexthop index if successful,
otherwise, a negative value is returned
*/
e3_type spine_api_register_mnexthop(e3_type e3api_service)
{
	int ret=register_multicast_nexthop();
	return ret;
}
DECLARE_E3_API(mnexthop_registeration)={
	.api_name="spine_api_register_mnexthop",
	.api_desc="register a multicast nexthops entry",
	.api_callback_func=(api_callback_func)spine_api_register_mnexthop,
	.args_desc={
		{.type=e3_arg_type_none,},
	},
};

e3_type spine_api_get_mnexthop(e3_type e3api_service,e3_type index,e3_type entry)
{

	int16_t _index=e3_type_to_uint16_t(index);
	struct multicast_next_hops * _entry=(struct multicast_next_hops *)e3_type_to_uint8_t_ptr(entry);
	struct multicast_next_hops * pmnexthop=NULL;
	__read_lock_mnexthop();
	pmnexthop=_find_mnext_hops(_index);
	if(pmnexthop)
		rte_memcpy(_entry,pmnexthop,sizeof(struct multicast_next_hops));
	__read_unlock_mnexthop();
	return pmnexthop?E3_OK:-E3_ERR_NOT_FOUND;
}
DECLARE_E3_API(mnexthop_regstrival)={
	.api_name="spine_api_get_mnexthop",
	.api_desc="retrieve an multicast nexthop set entry",
	.api_callback_func=(api_callback_func)spine_api_get_mnexthop,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct multicast_next_hops)},
		{.type=e3_arg_type_none,},
	},
};

e3_type spine_api_list_mnexthops(e3_type e3api_service,e3_type nr_index_entry,e3_type index_entries)
{
	int16_t * _nr_index_entry=(int16_t *)e3_type_to_uint8_t_ptr(nr_index_entry);
	int16_t * _index_entries=(int16_t *)e3_type_to_uint8_t_ptr(index_entries);
	int idx=0;
	int iptr=0;
	__read_lock_mnexthop();
	for(idx=0;idx<MAX_MULTICAST_NEXT_HOPS;idx++){
		if(!find_mnext_hops(idx))
			continue;
		_index_entries[iptr]=idx;
		iptr++;
	}
	__read_unlock_mnexthop();
	*_nr_index_entry=iptr;
	return E3_OK;
}
DECLARE_E3_API(mnexthops_index_enuumeration)={
	.api_name="spine_api_list_mnexthops",
	.api_desc="enumerate multicast next hops",
	.api_callback_func=(api_callback_func)spine_api_list_mnexthops,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=2},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(int16_t)*MAX_MULTICAST_NEXT_HOPS},
		{.type=e3_arg_type_none,},
	},
};

e3_type spine_api_delete_mnexthop(e3_type e3api_service,e3_type mindex)
{
	int16_t _mindex=e3_type_to_uint16_t(mindex);
	return delete_multicast_nexthop(_mindex);
}
DECLARE_E3_API(mnexthop_deletion)={
	.api_name="spine_api_delete_mnexthop",
	.api_desc="delete a multicast nexthop entry",
	.api_callback_func=(api_callback_func)spine_api_delete_mnexthop,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};
e3_type spine_api_register_or_delete_nexthop_in_mnexthop(e3_type e3api_service,
		e3_type mindex,
		e3_type is_to_register,
		e3_type nexthop,
		e3_type label_to_push)
{
	int16_t _mindex=e3_type_to_uint16_t(mindex);
	int8_t  _is_to_register=e3_type_to_uint8_t(is_to_register);
	int16_t _nexthop=e3_type_to_uint16_t(nexthop);
	int32_t _label_to_push=e3_type_to_uint32_t(label_to_push);
	if(_is_to_register)
		return register_nexthop_in_mnexthops(_mindex,_nexthop,_label_to_push);
	return delete_nexthop_in_mnexthops(_mindex,_nexthop,_label_to_push);
}
DECLARE_E3_API(nexthop_in_mnexthops_registeration)={
	.api_name="spine_api_register_or_delete_nexthop_in_mnexthop",
	.api_desc="register or delete a nexthop entry in mnexthops",
	.api_callback_func=(api_callback_func)spine_api_register_or_delete_nexthop_in_mnexthop,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none},
	},
};

