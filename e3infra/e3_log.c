/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3infra/include/e3_log.h>
#include <e3infra/include/e3_init.h>
#include <e3infra/include/e3-ini-config.h>
#include <rte_spinlock.h>
time_t log_time;
static rte_spinlock_t log_guard;

void lock_log(void *arg,int lock)
{
	if(lock)
		rte_spinlock_lock(&log_guard);
	else
		rte_spinlock_unlock(&log_guard);
}
void e3_log_module_init(void)
{
	int _log_level=LOG_INFO;
	FILE * fp_log=NULL;
	char * log_path=get_ini_option_string("default","log_path");
	char * log_level=get_ini_option_string("default","log_level");
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char s[64];
	strftime(s, sizeof(s), "%c", tm);

	rte_spinlock_init(&log_guard);
	if(!log_path)
		log_path=LOG_FILE_PATH;
	fp_log=fopen(log_path,"a+");
	if(!fp_log)
		fp_log=stderr;

	if(log_level){
		if(!strcmp(log_level,"debug")){
			_log_level=LOG_DEBUG;
		}else if(!strcmp(log_level,"info")){
			_log_level=LOG_INFO;
		}else if(!strcmp(log_level,"warn")){
			_log_level=LOG_WARN;
		}else if(!strcmp(log_level,"error")){
			_log_level=LOG_ERROR;
		}else if(!strcmp(log_level,"fatal")){
			_log_level=LOG_FATAL;
		}
	}
	log_set_fp(fp_log);
	log_set_quiet(1);
	log_set_level(_log_level);
	log_set_lock(lock_log);
	E3_LOG("\n----------log starts at:%s-----------\n",s);
}
E3_init(e3_log_module_init,TASK_PRIORITY_LOG_INIT);