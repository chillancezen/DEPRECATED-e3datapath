#include <e3net/include/common-nhlfe.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/util.h>
e3_type e3net_api_register_common_neighbor(e3_type e3service,
	e3_type ip_string,
	e3_type mac_string)
{
	char * _ip_string	=(char *)e3_type_to_uint8_t_ptr(ip_string);
	char * _mac_string	=(char *)e3_type_to_uint8_t_ptr(mac_string);

	struct common_neighbor  neighbor;
	neighbor.neighbour_ip_as_le=_ip_string_to_u32_le(_ip_string);
	_mac_string_to_byte_array(_mac_string,neighbor.mac);
	printf("hello worlf\n");
	return register_common_neighbor(&neighbor);
}
DECLARE_E3_API(common_neighbor_registeration)={
	.api_name="e3net_api_register_common_neighbor",
	.api_desc="register a common neighbor",
	.api_callback_func=(api_callback_func)e3net_api_register_common_neighbor,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=E3API_IP_STRING_LENGTH},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=E3API_MAC_STRING_LENGTH},
		{.type=e3_arg_type_none,},
	},
};
