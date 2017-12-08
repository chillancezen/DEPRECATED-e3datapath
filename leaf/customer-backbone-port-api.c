/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/leaf-e-service.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/util.h>
#include <rte_memcpy.h>
#include <leaf/include/customer-backbone-port.h>
#include <e3net/include/e3iface-inventory.h>
#include <leaf/include/leaf-label-fib.h>


e3_type leaf_api_cbp_setup_label_entry(e3_type e3service,
	e3_type iface_id,
	e3_type label_id,
	e3_type is_eline_service,
	e3_type service_id)
{
	int ret=E3_OK;
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int32_t _label_id=e3_type_to_uint16_t(label_id);
	int8_t  _is_eline_service=e3_type_to_uint8_t(is_eline_service);
	int16_t _service_id=e3_type_to_uint16_t(service_id);

	struct E3Interface * pif=NULL;
	struct cbp_private * priv=NULL;
	struct leaf_label_entry tmp_label_entry;
	tmp_label_entry.e3_service=_is_eline_service?e_line_service:e_lan_service;
	tmp_label_entry.service_index=_service_id;
	
	pif=find_e3interface_by_index(_iface_id);
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT)||
		(_label_id<0)||
		(_label_id>=NR_LEAF_LABEL_ENTRY))
		return -E3_ERR_ILLEGAL;
	priv=(struct cbp_private *)pif->private;
	rte_rwlock_write_lock(&priv->cbp_guard);
	/*
	*release previous  ingress binding for the lable
	*/
	if(priv->label_base[_label_id].is_valid){
		reset_leaf_label_entry(priv->label_base,_label_id);
	}
	ret=set_leaf_label_entry(priv->label_base,_label_id,&tmp_label_entry);
	rte_rwlock_write_unlock(&priv->cbp_guard);
	return ret; 
}
DECLARE_E3_API(cbp_nhlfe_setup)={
	.api_name="leaf_api_cbp_setup_label_entry",
	.api_desc="set a label entry of a customer backbone port",
	.api_callback_func=(api_callback_func)leaf_api_cbp_setup_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};

e3_type leaf_api_cbp_clear_label_entry(e3_type e3service,
	e3_type iface_id,
	e3_type label_id)
{
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int32_t _label_id=e3_type_to_uint16_t(label_id);

	struct E3Interface * pif=NULL;
	struct cbp_private * priv=NULL;
	pif=find_e3interface_by_index(_iface_id);
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT)||
		(_label_id<0)||
		(_label_id>=NR_LEAF_LABEL_ENTRY))
		return -E3_ERR_ILLEGAL;
	priv=(struct cbp_private *)pif->private;
	rte_rwlock_write_lock(&priv->cbp_guard);
	reset_leaf_label_entry(priv->label_base,_label_id);
	rte_rwlock_write_unlock(&priv->cbp_guard);
	return E3_OK;
}
DECLARE_E3_API(cbp_label_entry_reset)={
	.api_name="leaf_api_cbp_clear_label_entry",
	.api_desc="reset the label entry of a custoemr backbone port",
	.api_callback_func=(api_callback_func)leaf_api_cbp_clear_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};

e3_type leaf_api_cbp_get_label_entry(e3_type e3servie,
	e3_type iface_id,
	e3_type label_id,
	e3_type entry)
{
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int32_t _label_id=e3_type_to_uint32_t(label_id);
	struct leaf_label_entry * _entry=(struct leaf_label_entry*)e3_type_to_uint8_t_ptr(entry);
	struct E3Interface * pif=NULL;
	struct cbp_private * priv=NULL;
	
	pif=find_e3interface_by_index(_iface_id);
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT)||
		(_label_id<0)||
		(_label_id>=NR_LEAF_LABEL_ENTRY))
		return -E3_ERR_GENERIC;
	priv=(struct cbp_private*)pif->private;
	rte_rwlock_read_lock(&priv->cbp_guard);
	rte_memcpy(_entry,&priv->label_base[_label_id],sizeof(struct leaf_label_entry));
	rte_rwlock_read_unlock(&priv->cbp_guard);
	return E3_OK;
}
DECLARE_E3_API(cbp_label_entry_retrieval)={
	.api_name="leaf_api_cbp_get_label_entry",
	.api_desc="retrieve a label entry of customer backbone port",
	.api_callback_func=(api_callback_func)leaf_api_cbp_get_label_entry,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct leaf_label_entry)},
		{.type=e3_arg_type_none,},
	},
};

#define CBP_MAX_NR_ENTRIES_PER_FETCH 1024
e3_type leaf_api_cbp_list_label_entry_partial(e3_type e3service,
		e3_type iface_id,
		e3_type index_to_start,
		e3_type nr_entries,
		e3_type entries,
		e3_type index_etries)
{
	int16_t _iface_id			=e3_type_to_uint16_t(iface_id);
	int32_t * _index_to_start	=(int32_t *)e3_type_to_uint8_t_ptr(index_to_start);
	int16_t * _nr_entries		=(int16_t*)e3_type_to_uint8_t_ptr(nr_entries);
	struct leaf_label_entry * _entries=(struct leaf_label_entry*)e3_type_to_uint8_t_ptr(entries);
	int32_t * _index_entries	=(int32_t *)e3_type_to_uint8_t_ptr(index_etries);
	int idx=0;
	int iptr=0;
	struct E3Interface * pif=NULL;
	struct cbp_private * priv=NULL;
	pif=find_e3interface_by_index(_iface_id);
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT))
		return -E3_ERR_GENERIC;
	priv=(struct cbp_private*)pif->private;
	rte_rwlock_read_lock(&priv->cbp_guard);
	for(idx=*_index_to_start;idx<NR_LEAF_LABEL_ENTRY;idx++){
		if(!priv->label_base[idx].is_valid)
			continue;
		rte_memcpy(&_entries[iptr],&priv->label_base[idx],sizeof(struct leaf_label_entry));
		_index_entries[iptr]=idx;
		iptr++;
		if(iptr==CBP_MAX_NR_ENTRIES_PER_FETCH)
			break;
	}
	*_index_to_start=idx+1;
	*_nr_entries=iptr;
	rte_rwlock_read_unlock(&priv->cbp_guard);
	return E3_OK;
}
DECLARE_E3_API(cbp_label_entry_enumeration)={
	.api_name="leaf_api_cbp_list_label_entry_partial",
	.api_desc="retrieve a part of the customer backbone port's label entries",
	.api_callback_func=(api_callback_func)leaf_api_cbp_list_label_entry_partial,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=2},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(struct leaf_label_entry)*CBP_MAX_NR_ENTRIES_PER_FETCH},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(int32_t)*CBP_MAX_NR_ENTRIES_PER_FETCH},
		{.type=e3_arg_type_none,},
	},
};

/*
*it succeeds if and only if the label entry is mapped into a
*E-LAN service, and <nhlfe,label_to_push> is registered into E-LAN
*/
e3_type leaf_api_set_cbp_egress_nhlfe_index(e3_type e3service,
		e3_type iface_id,
		e3_type label_id,
		e3_type nhlfe,
		e3_type label_to_push)
{
	int ret=-E3_ERR_GENERIC;
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int32_t _label_id=e3_type_to_uint32_t(label_id);
	int16_t _nhlfe=e3_type_to_uint16_t(nhlfe);
	int32_t _label_to_push=e3_type_to_uint32_t(label_to_push);

	struct E3Interface * pif=NULL;
	struct cbp_private * priv=NULL;
	pif=find_e3interface_by_index(_iface_id);
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT)||
		(_label_id<0)||
		(_label_id>=NR_LEAF_LABEL_ENTRY))
		return -E3_ERR_ILLEGAL;
	priv=(struct cbp_private *)pif->private;
	rte_rwlock_write_lock(&priv->cbp_guard);
	ret=set_leaf_label_entry_egress_nhlfe_index(priv->label_base,
		_label_id,
		_nhlfe,
		_label_to_push);
	rte_rwlock_write_unlock(&priv->cbp_guard);
	return ret;
}
DECLARE_E3_API(cbp_egress_nhlfe_index_setup)={
	.api_name="leaf_api_set_cbp_egress_nhlfe_index",
	.api_desc="set the egress nhlfe index of an label entry of customer backbone port",
	.api_callback_func=(api_callback_func)leaf_api_set_cbp_egress_nhlfe_index,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};

e3_type leaf_api_clear_cbp_egress_nhlfe_index(e3_type e3service,
	e3_type iface_id,
	e3_type label_id)
{
	int ret=-E3_ERR_GENERIC;
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int32_t _label_id=e3_type_to_uint32_t(label_id);
	struct E3Interface * pif=NULL;
	struct cbp_private * priv=NULL;
	pif=find_e3interface_by_index(_iface_id);
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT)||
		(_label_id<0)||
		(_label_id>=NR_LEAF_LABEL_ENTRY))
		return -E3_ERR_ILLEGAL;
	priv=(struct cbp_private *)pif->private;
	rte_rwlock_write_lock(&priv->cbp_guard);
	ret=clear_leaf_label_entry_egress_nhlfe_index(priv->label_base,
		_label_id);
	rte_rwlock_write_unlock(&priv->cbp_guard);
	return ret;
}

DECLARE_E3_API(cbp_egress_nhlfe_index_cleanup)={
	.api_name="leaf_api_clear_cbp_egress_nhlfe_index",
	.api_desc="reset the egress nhlfe index of an label entry of customer backbone port",
	.api_callback_func=(api_callback_func)leaf_api_clear_cbp_egress_nhlfe_index,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};