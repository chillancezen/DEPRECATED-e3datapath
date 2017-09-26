#include <e3-api.h>
#include <string.h>

struct e3_api_declaration *e3_api_head=(void*)0;

struct e3_api_declaration * search_e3_api_by_name(const char * api_name)
{
	struct e3_api_declaration * api_ptr=e3_api_head;
	for(;api_ptr;api_ptr=api_ptr->next){
		if(!strcmp(api_ptr->api_name,api_name))
			break;
	}
	return api_ptr;
}
/*seach and verify the api declaration*/
struct e3_api_declaration * search_e3_api_by_template(struct e3_api_declaration * api_template)
{
	int idx=0;
	struct e3_api_declaration * api=search_e3_api_by_name(api_template->api_name);
	if((!api) || (!api->api_callback_func))
		return NULL;
	/*arguments types and behaviors ,
	even length must be consistent*/
	for(idx=0;idx<MAX_ARGUMENT_SUPPORTED;idx++){
		if((api->args_desc[idx].type==e3_arg_type_none)&&
			(api_template->args_desc[idx].type==e3_arg_type_none))
			break;
		if((api->args_desc[idx].type!=api_template->args_desc[idx].type)||
			(api->args_desc[idx].behavior!=api_template->args_desc[idx].behavior))
			return NULL;
		if((api->args_desc[idx].type==e3_arg_type_uint8_t_ptr)&&
			(api->args_desc[idx].len!=api_template->args_desc[idx].len))
			return NULL;
	}
	return api;
}
int e3_api_nr_args(struct e3_api_declaration * api)
{
	int nr_arg=0;
	int idx=0;
	for(idx=0;idx<MAX_ARGUMENT_SUPPORTED;idx++){
		if(api->args_desc[idx].type==e3_arg_type_none)
			break;
		nr_arg++;
	}
	return nr_arg;
}

