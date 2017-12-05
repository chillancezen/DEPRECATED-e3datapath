/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/leaf-e-service.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/util.h>
#include <rte_memcpy.h>

/*
*export APIs for E-LINE and E-LAN servces
*note:these APIs are designed to support multi-threading
*/

/*
*this API register an ether service service
*it applies to both e-line and e-lan 
*/

e3_type leaf_api_register_e_service(e3_type e3service,e3_type is_eline,e3_type pservice_id)
{
	uint8_t    _is_eline=e3_type_to_uint8_t(is_eline);
	uint32_t * _pservice_id=(uint32_t*)e3_type_to_uint8_t_ptr(pservice_id);
	int ret=-E3_ERR_GENERIC;
	if(_is_eline)
		ret=register_e_line_service();
	else
		ret=register_e_lan_service();
	
	if(ret<0)
		return -E3_ERR_OUT_OF_RES;
	*_pservice_id=ret;
	return E3_OK;
}

DECLARE_E3_API(e_service_registration)={
	.api_name="leaf_api_register_e_service",
	.api_desc="register an Ether-service",
	.api_callback_func=(api_callback_func)leaf_api_register_e_service,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=4},
		{.type=e3_arg_type_none,},
	},
};


/*
*E-LINE only
*/
e3_type leaf_api_get_e_line(e3_type e3service,e3_type eline_index,e3_type peline)
{
	uint32_t _eline_index=e3_type_to_uint32_t(eline_index);
	struct ether_e_line *_peline=(struct ether_e_line *)e3_type_to_uint8_t_ptr(peline);
	__read_lock_eline();
	#if 0
	struct ether_e_line *eline=find_e_line_service(_eline_index);
	#else
	/*
	*here does not check the validation bit of e-line service,
	*only give it to the other end of API calling
	*/
	struct ether_e_line *eline=_find_e_line_service(_eline_index);
	#endif
	if(!eline){
		__read_unlock_eline();
		return -E3_ERR_NOT_FOUND;
	}
	rte_memcpy(_peline,eline,sizeof(struct ether_e_line));
	__read_unlock_eline();
	return E3_OK;
}
DECLARE_E3_API(e_line_retrieval)={
	.api_name="leaf_api_get_e_line",
	.api_desc="copy an e-line to other client endpoit",
	.api_callback_func=(api_callback_func)leaf_api_get_e_line,
	.args_desc={
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct ether_e_line)},
		{.type=e3_arg_type_none},
	},
};

/*
*this api applies to E-LAN and E-LINE
*/
#if MAX_E_LINE_SERVICES!=MAX_E_LAN_SERVICES
#error "MAX_E_LINE_SERVICES must be equal to MAX_E_LINE_SERVICES"
#endif

e3_type leaf_api_list_e_services(e3_type e3service,e3_type is_eline,e3_type pnr_services,e3_type pservices)
{
	uint8_t    _is_eline    =e3_type_to_uint8_t(is_eline);
	uint32_t * _pnr_services=(uint32_t *)e3_type_to_uint8_t_ptr(pnr_services);
	uint16_t * _pservices   =(uint16_t *)e3_type_to_uint8_t_ptr(pservices);
	
	struct ether_e_line * eline;
	struct ether_e_lan * elan;
	int idx=0;
	int nr_services=0;
	
	if(_is_eline){
		__read_lock_eline();
		for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
			if(!(eline=find_e_line_service(idx)))
				continue;
			_pservices[nr_services]=idx;
			nr_services+=1;
		}
		__read_unlock_eline();
	}else{
		__read_lock_elan();
		for(idx=0;idx<MAX_E_LAN_SERVICES;idx++){
			if(!(elan=find_e_lan_service(idx)))
				continue;
			_pservices[nr_services]=idx;
			nr_services+=1;
		}
		__read_unlock_elan();
	}
	*_pnr_services=nr_services;
	return E3_OK;
}

DECLARE_E3_API(e_services_list)={
	.api_name="leaf_api_list_e_services",
	.api_desc="get the list of ether services",
	.api_callback_func=(api_callback_func)leaf_api_list_e_services,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=2*MAX_E_LINE_SERVICES},
		{.type=e3_arg_type_none},
	},
};
/*
*this api routine applies to both E-LINE and E-LAN services
*/
e3_type leaf_api_delete_e_service(e3_type e3service,e3_type is_eline,e3_type service_index)
{
	int8_t 	_is_eline	=e3_type_to_uint8_t(is_eline);
	int32_t _service_index=e3_type_to_uint32_t(service_index);
	if(_is_eline)
		return delete_e_line_service(_service_index);
	return delete_e_lan_service(service_index);
}
DECLARE_E3_API(e_service_deletion)={
	.api_name="leaf_api_delete_e_service",
	.api_desc="deltee an ether service",
	.api_callback_func=(api_callback_func)leaf_api_delete_e_service,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};

/*
*E-LAN&E-LINE available,
*however, they should be treated seperatedly since their
*return values have completely different meanings.
*/
e3_type leaf_api_register_e_service_port(e3_type e3service,
	e3_type is_eline,
	e3_type service_index,
	e3_type e3iface_index,
	e3_type vlan_tci)
{
	int8_t  _is_eline=e3_type_to_uint8_t(is_eline);
	int16_t _service_index=e3_type_to_uint16_t(service_index);
	int32_t _e3iface_index=e3_type_to_uint32_t(e3iface_index);
	int32_t _vlan_tci=e3_type_to_uint32_t(vlan_tci);
	if(_is_eline)
		return register_e_line_port(_service_index,_e3iface_index,_vlan_tci);
	return register_e_lan_port(_service_index,_e3iface_index,_vlan_tci);
}
DECLARE_E3_API(service_port_registration)={
	.api_name="leaf_api_register_e_service_port",
	.api_desc="register a port for both e-lan&e-line services",
	.api_callback_func=(api_callback_func)leaf_api_register_e_service_port,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	},
};
/*
*E-LAN&E-LINE capable
*/
e3_type leaf_api_delete_e_service_port(e3_type e3service,
	e3_type is_eline,
	e3_type service_index,
	e3_type inner_port_id)
{
	int8_t  _is_eline=e3_type_to_uint8_t(is_eline);
	int16_t _service_index=e3_type_to_uint16_t(service_index);
	int16_t _inner_port_id=e3_type_to_uint16_t(inner_port_id);
	if(_is_eline)
		return delete_e_line_port(_service_index);
	return delete_e_lan_port(_service_index,_inner_port_id);
}
DECLARE_E3_API(service_port_deletion)={
	.api_name="leaf_api_delete_e_service_port",
	.api_desc="delete an ether service port",
	.api_callback_func=(api_callback_func)leaf_api_delete_e_service_port,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,},
	}
};
/*
*both e-line&e-lan capable
*/
e3_type leaf_api_register_e_service_nhlfe(e3_type e3service,
	e3_type is_eline,
	e3_type service_index,
	e3_type nhlfe,
	e3_type label_to_push)
{
	int8_t  _is_eline		=e3_type_to_uint8_t(is_eline);
	int16_t _service_index	=e3_type_to_uint16_t(service_index);
	int32_t _nhlfe			=e3_type_to_uint32_t(nhlfe);
	int32_t _label_to_push	=e3_type_to_uint32_t(label_to_push);
	if(_is_eline)
		return register_e_line_nhlfe(_service_index,_nhlfe,_label_to_push);
	return register_e_lan_nhlfe(_service_index,_nhlfe,_label_to_push);
}
DECLARE_E3_API(service_nhlfe_registeration)={
	.api_name="leaf_api_register_e_service_nhlfe",
	.api_desc="register a nhlfe for both ether service",
	.api_callback_func=(api_callback_func)leaf_api_register_e_service_nhlfe,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint32_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,}
	},
};

/*
*both e-line&e-lan capable
*/
e3_type leaf_api_delete_e_service_nhlfe(e3_type service,
	e3_type is_eline,
	e3_type service_index,
	e3_type inner_nhlfe_index)
{
	int8_t _is_eline=e3_type_to_uint8_t(is_eline);
	int16_t _service_index=e3_type_to_uint16_t(service_index);
	int16_t _inner_nhlfe_index=e3_type_to_uint16_t(inner_nhlfe_index);
	if(_is_eline)
		return delete_e_line_nhlfe(_service_index);
	return delete_e_lan_nhlfe(_service_index,_inner_nhlfe_index);
}
DECLARE_E3_API(service_nhlfe_deletion)={
	.api_name="leaf_api_delete_e_service_nhlfe",
	.api_desc="delete a nhlfe of an ether service",
	.api_callback_func=(api_callback_func)leaf_api_delete_e_service_nhlfe,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_none,}
	},
};


e3_type leaf_api_get_e_lan(e3_type e3service,
	e3_type elan_index,
	e3_type pelan)
{
	int16_t _elan_index=e3_type_to_uint16_t(elan_index);
	struct ether_e_lan * _pelan=(struct ether_e_lan*)e3_type_to_uint8_t_ptr(pelan);
	struct ether_e_lan * elan=NULL;
	__read_lock_elan();
	elan=_find_e_lan_service(_elan_index);
	__read_unlock_elan();
	if(elan){
		rte_memcpy(_pelan,elan,sizeof(struct ether_e_lan));
		return E3_OK;
	}
	return -E3_ERR_NOT_FOUND;
}
DECLARE_E3_API(e_lan_retrieval)={
	.api_name="leaf_api_get_e_lan",
	.api_desc="retrieve an e-lan service entry",
	.api_callback_func=(api_callback_func)leaf_api_get_e_lan,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct ether_e_lan)},
		{.type=e3_arg_type_none,},
	},	
};

