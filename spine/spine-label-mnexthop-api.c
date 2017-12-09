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
	int ret=register_multicast_nexthops();
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

