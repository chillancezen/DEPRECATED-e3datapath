/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/leaf-e-service.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/util.h>
#include <rte_memcpy.h>
#include <leaf/include/customer-service-port.h>
#include <e3net/include/e3iface-inventory.h>
#include <leaf/include/leaf-label-fib.h>
#include <e3infra/include/e3-log.h>


e3_type leaf_api_csp_setup_port(e3_type e3service,
	e3_type iface_id,
	e3_type vlan_tci,
	e3_type is_eline_service,
	e3_type service_id)
{
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int16_t _vlan_tci=e3_type_to_uint16_t(vlan_tci);
	int8_t  _is_eline_service=e3_type_to_uint8_t(is_eline_service);
	int16_t _service_id=e3_type_to_uint16_t(service_id);

	int ret=-E3_ERR_GENERIC;
	struct E3Interface *pif=find_e3interface_by_index(_iface_id);
	struct csp_private *priv=NULL;
	if((!pif)||(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT))
		return -E3_ERR_NOT_SUPPORTED;
	if((vlan_tci<0)||(vlan_tci>=4096))
		return -E3_ERR_ILLEGAL;
	
	priv=(struct csp_private*)pif->private;
	
	rte_rwlock_write_lock(&priv->csp_guard);
	/*
	*if the e-line service is already used, user should mannually release it
	*/
	if(_is_eline_service){
		struct ether_e_line *_eline=find_e_line_service(_service_id);
		if(_eline&&_eline->is_csp_ready){
			ret=-E3_ERR_IN_USE;
			goto out;
		}
	}
	/*
	*release previous binding first
	*/
	if(priv->vlans[_vlan_tci].is_valid){
		switch(priv->vlans[_vlan_tci].e_service)
		{
			case e_line_service:
				if(delete_e_line_port(priv->vlans[_vlan_tci].service_index)){
					E3_WARN("can not delete the port of e-line:%d\n",priv->vlans[_vlan_tci].service_index);
				}
				break;
			case e_lan_service:
				{
					int inner_port_id=find_e_lan_port_locked(priv->vlans[_vlan_tci].service_index,
							_iface_id,
							_vlan_tci);
					if(inner_port_id<0)
						break;
					if(delete_e_lan_port(priv->vlans[_vlan_tci].service_index,inner_port_id)){
						E3_WARN("can not delete the port:%d of e-lan:%d\n",
							inner_port_id,
							priv->vlans[_vlan_tci].service_index);
					}
				}
				break;
			default:
				break;
		}
		priv->vlans[_vlan_tci].is_valid=0;
	}
	/*
	*set up the relationship
	*/
	if(_is_eline_service){
		priv->vlans[_vlan_tci].e_service=e_line_service;
		priv->vlans[_vlan_tci].service_index=_service_id;
		if(register_e_line_port(_service_id,_iface_id,_vlan_tci))
			goto out;
		priv->vlans[_vlan_tci].is_valid=1;
	}else{
		priv->vlans[_vlan_tci].e_service=e_lan_service;
		priv->vlans[_vlan_tci].service_index=_service_id;
		if(register_e_lan_port(_service_id,_iface_id,_vlan_tci)<0)
			goto out;
		priv->vlans[_vlan_tci].is_valid=1;
	}
	ret=E3_OK;
	out:
	rte_rwlock_write_unlock(&priv->csp_guard);
	E3_LOG("register port %d's vlan entry %d to %s service %d with result as:%d\n",
			_iface_id,
			_vlan_tci,
			_is_eline_service?"E-LINE":"E-LAN",
			_service_id,
			ret);
	return ret;
}
DECLARE_E3_API(csp_port_setup)={
	.api_name="leaf_api_csp_setup_port",
	.api_desc="setup the csp port vlan entry",
	.api_callback_func=(api_callback_func)leaf_api_csp_setup_port,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};

e3_type leaf_api_csp_withdraw_port(e3_type e3service,
		e3_type iface_id,
		e3_type vlan_tci)
{
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int16_t _vlan_tci=e3_type_to_uint16_t(vlan_tci);

	struct E3Interface *pif=find_e3interface_by_index(_iface_id);
	struct csp_private *priv=NULL;
	if((!pif)||(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT))
		return -E3_ERR_NOT_SUPPORTED;
	if((vlan_tci<0)||(vlan_tci>=4096))
		return -E3_ERR_ILLEGAL;
	priv=(struct csp_private*)pif->private;
	rte_rwlock_write_lock(&priv->csp_guard);
	if(priv->vlans[_vlan_tci].is_valid){
		switch(priv->vlans[_vlan_tci].e_service)
		{
			case e_line_service:
				if(delete_e_line_port(priv->vlans[_vlan_tci].service_index)){
					E3_WARN("can not delete the port of e-line:%d\n",priv->vlans[_vlan_tci].service_index);
				}
				break;
			case e_lan_service:
				{
					int inner_port_id=find_e_lan_port_locked(priv->vlans[_vlan_tci].service_index,
							_iface_id,
							_vlan_tci);
					if(inner_port_id<0)
						break;
					if(delete_e_lan_port(priv->vlans[_vlan_tci].service_index,inner_port_id)){
						E3_WARN("can not delete the port:%d of e-lan:%d\n",
							inner_port_id,
							priv->vlans[_vlan_tci].service_index);
					}
				}
				break;
			default:
				break;
		}
	}
	priv->vlans[_vlan_tci].is_valid=0;
	rte_rwlock_write_unlock(&priv->csp_guard);
	E3_LOG("withdraw port %d's vlan %d's entry\n",
		_iface_id,
		_vlan_tci);
	
	return E3_OK;
}
DECLARE_E3_API(csp_port_withdrawal)={
	.api_name="leaf_api_csp_withdraw_port",
	.api_desc="withdraw a vlan entry of a port from e-service",
	.api_callback_func=(api_callback_func)leaf_api_csp_withdraw_port,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};
#define CSP_TABLE_NR_ENTRIES_PER_FETCH 2048
#define CSP_TABLE_NR_BLOCK (4096/CSP_TABLE_NR_ENTRIES_PER_FETCH)

e3_type leaf_api_list_csp_distribution_table(e3_type e3service,
		e3_type iface_id,
		e3_type block_index,
		e3_type entries)
{
	int16_t _iface_id=e3_type_to_uint16_t(iface_id);
	int16_t _block_index=e3_type_to_uint16_t(block_index);
	struct csp_distribution_entry * _entries=(struct csp_distribution_entry*)
			e3_type_to_uint8_t_ptr(entries);
	
	struct E3Interface *pif=find_e3interface_by_index(_iface_id);
	struct csp_private *priv=NULL;
	if((!pif)||
		(pif->hwiface_role!=E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT)||
		(_block_index<0)||
		(_block_index>=CSP_TABLE_NR_BLOCK))
		return -E3_ERR_NOT_SUPPORTED;
	priv=(struct csp_private*)pif->private;
	rte_rwlock_read_lock(&priv->csp_guard);
	rte_memcpy(_entries,
		&priv->vlans[CSP_TABLE_NR_ENTRIES_PER_FETCH*_block_index],
		sizeof(struct csp_distribution_entry)*CSP_TABLE_NR_ENTRIES_PER_FETCH);
	rte_rwlock_read_unlock(&priv->csp_guard);
	return E3_OK;
}
DECLARE_E3_API(csp_dist_tbl_enumeration)={
	.api_name="leaf_api_list_csp_distribution_table",
	.api_desc="fetch cs port's distribution table",
	.api_callback_func=(api_callback_func)leaf_api_list_csp_distribution_table,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
			.len=sizeof(struct csp_distribution_entry)*CSP_TABLE_NR_ENTRIES_PER_FETCH},
		{.type=e3_arg_type_none,},
	},
};
