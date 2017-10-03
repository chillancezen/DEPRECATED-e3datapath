#include <e3-api-export.h>
#include <assert.h>

int get_file_handler(const char * file_header,
						const char * file_source,
						FILE **fp_header,
						FILE **fp_source)
{
	FILE * _fp_header=NULL;
	FILE * _fp_source=NULL;
	_fp_header=fopen(file_header,"w+");
	_fp_source=fopen(file_source,"w+");

	if((!_fp_header)||(!_fp_source)){

		if(_fp_header)
			fclose(_fp_header);
		if(_fp_source)
			fclose(_fp_source);
		return -1;
	}
	*fp_header=_fp_header;
	*fp_source=_fp_source;
	return 0;
}


int init_api_source_file(FILE * fp)
{
	fprintf(fp,"#include <exported-api.h>\n");
	fprintf(fp,"#include <e3-api-wrapper.h>\n");
	fflush(fp);
	
	return 0;
}



static int _generate_api_declaration_argument(struct e3_api_declaration * api,FILE * fp)
{
	int idx=0;
	fprintf(fp,"\t.args_desc={\n");
	for(idx=0;idx<(MAX_ARGUMENT_SUPPORTED+1);idx++){
		fprintf(fp,"\t\t{.type=%s,.behavior=%s,.len=%d},\n",
						e3_arg_type_to_string(api->args_desc[idx].type),
						e3_arg_behavior_to_string(api->args_desc[idx].behavior),
						api->args_desc[idx].len);

		if(api->args_desc[idx].type==e3_arg_type_none)
			break;
	}
	fprintf(fp,"\t},\n");
	return 0;
}
/*
*generate api declaration in a source file
*/
int generate_api_declaration(FILE * fp)
{
	struct e3_api_declaration * api=e3_api_head;
	int e3_api_cnt=0;
	for(;api;api=api->next,e3_api_cnt++){
		fprintf(fp,"DECLARE_E3_API(e3_exported_api%d)={\n",e3_api_cnt);
		fprintf(fp,"\t.api_name=\"%s\",\n",api->api_name);
		fprintf(fp,"\t.api_desc=\"%s\",\n",api->api_desc);
		_generate_api_declaration_argument(api,fp);
		fprintf(fp,"};\n");
	}

	fflush(fp);
	return 0;
}

static int _generate_api_function_name(struct e3_api_declaration * api,FILE * fp)
{
	int idx=0;
	int arg_cnt=0;
	
	fprintf(fp,"uint64_t %s(uint64_t * api_ret",api->api_name);
	for(idx=0;idx<(MAX_ARGUMENT_SUPPORTED+1);idx++,arg_cnt++){
		if(api->args_desc[idx].type==e3_arg_type_none)
			break;
		fprintf(fp,",\n\t%s arg%d",e3_arg_type_to_std_type(api->args_desc[idx].type),arg_cnt);
		
	}
	fprintf(fp,")\n");
	fflush(fp);
	return 0;
}
static int _generate_api_funtion_arg_list(struct e3_api_declaration * api,FILE * fp)
{
	int idx=0;
	int arg_cnt=0;
	
	
	for(idx=0;idx<(MAX_ARGUMENT_SUPPORTED+1);idx++,arg_cnt++){
		if(api->args_desc[idx].type==e3_arg_type_none)
			break;
		fprintf(fp,"\treal_args[%d]=cast_to_e3_type(arg%d);\n",arg_cnt,arg_cnt);
	}
	
	fprintf(fp,"\toutput_list[0]=api_ret;\n");
	for(idx=0,arg_cnt=1;idx<(MAX_ARGUMENT_SUPPORTED+1);idx++){
		if(api->args_desc[idx].type==e3_arg_type_none)
			break;
		switch(api->args_desc[idx].behavior)
		{
			case e3_arg_behavior_output:
			case e3_arg_behavior_input_and_output:
				fprintf(fp,"\toutput_list[%d]=arg%d;\n",arg_cnt++,idx);
			break;
			default:
			break;
		}
	}
	fprintf(fp,"\tclient->para_output_list=output_list;\n");
	fprintf(fp,"\tclient->nr_output_list=%d;\n",arg_cnt);
	return 0;
}
int generate_api_function_body(FILE * fp)
{
	struct e3_api_declaration * api=e3_api_head;
	for(;api;api=api->next){
		_generate_api_function_name(api,fp);
		fprintf(fp,"{\n");
		fprintf(fp,"\t#define _(con) if(!(con)) goto error;\n");
		fprintf(fp,"\te3_type real_args[MAX_ARGUMENT_SUPPORTED];\n"
			"\tstruct e3_api_client      * client=reference_e3_api_client();\n"
			"\tstruct e3_api_declaration * api=search_e3_api_by_name(\"%s\");\n"
			"\tvoid                      * output_list[MAX_ARGUMENT_SUPPORTED+1];\n",
			api->api_name);
		fprintf(fp,"\t_(client);\n"
			"\t_(api);\n");
		_generate_api_funtion_arg_list(api,fp);

		fprintf(fp,"\t_(!encode_e3_api_request(client->send_mbuf,MAX_MSG_LENGTH,api,real_args));\n");
		fprintf(fp,"\t_(!issue_e3_api_request(client));\n");
		fprintf(fp,"\tdereference_e3_api_client(client);\n"
			"\treturn 0;\n"
			"\terror:\n"
			"\t\tif(client)\n"
			"\t\t\tdereference_e3_api_client(client);\n"
			"\t\treturn -1;\n");
		fprintf(fp,"\t#undef _\n");
		fprintf(fp,"}\n");
		fflush(fp);
	}

	return 0;
}

int generate_header_file(FILE * fp)
{
	int idx=0;
	int arg_cnt=0;
	
	fprintf(fp,"#include<stdint.h>\n");
	struct e3_api_declaration *api=e3_api_head;
	for(;api;api=api->next){
		fprintf(fp,"uint64_t %s(uint64_t * api_ret",api->api_name);
		for(idx=0;idx<(MAX_ARGUMENT_SUPPORTED+1);idx++,arg_cnt++){
			if(api->args_desc[idx].type==e3_arg_type_none)
				break;
			fprintf(fp,",\n\t%s arg%d",e3_arg_type_to_std_type(api->args_desc[idx].type),arg_cnt);
		}
		fprintf(fp,");\n");
	}
	fflush(fp);
	return 0;
}
/*test only*/

#if 0
DECLARE_E3_API(l2_api)={
	.api_name="l2_add_fib",
	.api_callback_func=(api_callback_func)NULL,
	.args_desc={
		{.type=e3_arg_type_uint8_t,.behavior=e3_arg_behavior_input},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=64},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=8},
		{.type=e3_arg_type_none},
	}
};
#endif

void e3_api_export_module_test(void)
{
	FILE * fp_hdr;
	FILE * fp_src;
	assert(!get_file_handler("./build/e3-hdr.h","./build/e3-src.c",&fp_hdr,&fp_src));
	init_api_source_file(fp_src);
	generate_api_declaration(fp_src);
	generate_api_function_body(fp_src);
	generate_header_file(fp_hdr);
}
