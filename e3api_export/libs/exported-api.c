#include <exported-api.h>
#include <e3-api-wrapper.h>
DECLARE_E3_API(e3_exported_api0)={
	.api_name="e3datapath_version",
	.api_desc="retrieve e3 datapath version dword",
	.args_desc={
		{.type=e3_arg_type_none,.behavior=e3_arg_behavior_none,.len=0},
	},
};
uint64_t e3datapath_version(uint64_t * api_ret)
{
	#define _(con) if(!(con)) goto error;
	e3_type real_args[MAX_ARGUMENT_SUPPORTED];
	struct e3_api_client      * client=reference_e3_api_client();
	struct e3_api_declaration * api=search_e3_api_by_name("e3datapath_version");
	void                      * output_list[MAX_ARGUMENT_SUPPORTED+1];
	_(client);
	_(api);
	output_list[0]=api_ret;
	client->para_output_list=output_list;
	client->nr_output_list=1;
	_(!encode_e3_api_request(client->send_mbuf,MAX_MSG_LENGTH,api,real_args));
	_(!issue_e3_api_request(client));
	dereference_e3_api_client(client);
	return 0;
	error:
		if(client)
			dereference_e3_api_client(client);
		return -1;
	#undef _
}
