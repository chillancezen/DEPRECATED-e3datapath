/*
*Copyright (c) 2016-2017 Jie Zheng
*/

#include <e3infra/include/e3-ini-config.h>
#include <stdio.h>
#include <stdlib.h>
ini_t * g_ini=NULL;
__attribute__((constructor))
	static void e3_ini_config_init(void)
{
	ini_t * _ini=ini_load(E3_DATAPATH_CONFIG_FILE);
	if(!_ini){
		/*
		*if the config file is not ready,let it be NULL
		*other option reader should be aware of this.
		*/
		fprintf(stderr,"[e3datapath]can not load ini file:%s\n",E3_DATAPATH_CONFIG_FILE);
	}
	g_ini=_ini;
}

char * get_ini_option_string(const char* section,const char * key)
{
	if(!g_ini)
		return NULL;
	return (char*)ini_get(g_ini,section,key);
}
