#ifndef E3_API_EXPORT_H
#define E3_API_EXPORT_H
#include <stdio.h>
#include <e3-api-wrapper.h>

int get_file_handler(const char * file_header,
						const char * file_source,
						FILE **fp_header,
						FILE **fp_source);
int init_api_source_file(FILE * fp);
int generate_api_declaration(FILE * fp);
int generate_api_function_body(FILE * fp);
int generate_header_file(FILE * fp);
void e3_api_export_module_test(void);

#endif 
