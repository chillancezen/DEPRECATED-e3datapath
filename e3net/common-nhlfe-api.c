/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3net/include/common-nhlfe.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/util.h>
#include <rte_memcpy.h>

e3_type e3net_api_register_or_update_common_neighbor(e3_type e3service,
	e3_type is_to_register,
	e3_type ip_string,
	e3_type mac_string)
{
	uint8_t _is_to_register	=e3_type_to_uint8_t(is_to_register);
	char * _ip_string		=(char *)e3_type_to_uint8_t_ptr(ip_string);
	char * _mac_string		=(char *)e3_type_to_uint8_t_ptr(mac_string);

	struct common_neighbor  neighbor;
	neighbor.neighbour_ip_as_le=_ip_string_to_u32_le(_ip_string);
	_mac_string_to_byte_array(_mac_string,neighbor.mac);
	if(_is_to_register)
		return register_common_neighbor(&neighbor);
	return refresh_common_neighbor_mac(&neighbor);
}
DECLARE_E3_API(common_neighbor_registeration)={
	.api_name="e3net_api_register_or_update_common_neighbor",
	.api_desc="register or update a common neighbor",
	.api_callback_func=(api_callback_func)e3net_api_register_or_update_common_neighbor,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=E3API_IP_STRING_LENGTH},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=E3API_MAC_STRING_LENGTH},
		{.type=e3_arg_type_none,},
	},
};

e3_type e3net_api_get_common_neighbor(e3_type e3service,e3_type neighbor_index,e3_type pneighbor)
{
	int16_t _neighbor_index=e3_type_to_uint16_t(neighbor_index);
	struct common_neighbor *_pneighbor=(struct common_neighbor*)e3_type_to_uint8_t_ptr(pneighbor);
	struct common_neighbor * neighbor=NULL;
	__read_lock_neighbor();
	#if 0
	neighbor=find_common_neighbor(_neighbor_index);
	#else
	neighbor=_find_common_neighbor(_neighbor_index);
	#endif
	if(!neighbor){
		__read_unlock_neighbor();
		return -E3_ERR_NOT_FOUND;
	}
	rte_memcpy(_pneighbor,neighbor,sizeof(struct common_neighbor));
	__read_unlock_neighbor();
	return E3_OK;
}
DECLARE_E3_API(common_neighbor_retrieval)={
	.api_name="e3net_api_get_common_neighbor",
	.api_desc="retrieve a common neighbor",
	.api_callback_func=(api_callback_func)e3net_api_get_common_neighbor,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct common_neighbor)},
		{.type=e3_arg_type_none},
	},
};
#define MAX_NR_NEIGHBORS_PER_FETCH 128

e3_type e3net_api_list_common_neighbor_partial(e3_type e3service,e3_type index_to_start,e3_type nr_entries,e3_type entries)
{
	int16_t *_index_to_start=(int16_t *)e3_type_to_uint8_t_ptr(index_to_start);
	int16_t *_nr_entries=(int16_t *)e3_type_to_uint8_t_ptr(nr_entries);
	struct common_neighbor * _entries=(struct common_neighbor*)e3_type_to_uint8_t_ptr(entries);
	struct common_neighbor * neighbor=NULL;
	int idx=0;
	int iptr=0;
	__read_lock_neighbor();
	for(idx=*_index_to_start;idx<MAX_COMMON_NEIGHBORS;idx++){
		if(!(neighbor=find_common_neighbor(idx)))
			continue;
		rte_memcpy(&_entries[iptr++],
			neighbor,
			sizeof(struct common_neighbor));
		if(iptr==MAX_NR_NEIGHBORS_PER_FETCH)
			break;
	}
	*_index_to_start=idx+1;
	*_nr_entries=iptr;
	__read_unlock_neighbor();
	return E3_OK;
}
DECLARE_E3_API(common_neighbors_enumeration)={
	.api_name="e3net_api_list_common_neighbor_partial",
	.api_desc="retrieve part of the neighbors set",
	.api_callback_func=(api_callback_func)e3net_api_list_common_neighbor_partial,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=2},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=2},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct common_neighbor)*MAX_NR_NEIGHBORS_PER_FETCH},
		{.type=e3_arg_type_none,},
	},
};

e3_type e3net_api_delete_common_neighbor(e3_type e3service,e3_type neighbor_index)
{
	int16_t _neighbor_index=e3_type_to_uint16_t(neighbor_index);
	return delete_common_neighbor(_neighbor_index);
}
DECLARE_E3_API(common_neighbor_deletion)={
	.api_name="e3net_api_delete_common_neighbor",
	.api_desc="delete a common neighbour",
	.api_callback_func=(api_callback_func)e3net_api_delete_common_neighbor,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};
