/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <stdio.h>
#include <e3-api-export.h>
#include <assert.h>

#define EXPORTED_HEADER "./libs/exported-api.h"
#define EXPORTED_SOURCE "./libs/exported-api.c"

int main(int argc,char ** argv)
{
	FILE * fp_hdr;
	FILE * fp_src;
	assert(!get_file_handler(EXPORTED_HEADER,EXPORTED_SOURCE,&fp_hdr,&fp_src));
	init_api_source_file(fp_src);
	generate_api_declaration(fp_src);
	generate_api_function_body(fp_src);
	generate_header_file(fp_hdr);
	fclose(fp_hdr);
	fclose(fp_src);
	return 0;
}
