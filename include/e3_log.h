#ifndef _E3_LOG_H
#define _E3_LOG_H

#include <stdio.h>
#include <time.h>
#include <string.h>


#define current_time() ({\
	int  _iptr=0; \
	char * _lptr; \
	time(&log_time); \
	_lptr=ctime(&log_time); \
	for(;_iptr<64;_iptr++) \
		if(_lptr[_iptr]=='\n'){ \
			_lptr[_iptr]='\0'; \
			break; \
		} \
	_lptr; \
})

#define E3_ASSERT(condition)  \
do{ \
        if(!(condition)){\
                fprintf(fp_log,"%s [assert] %s:%d %s() %s\n",current_time(),__FILE__,__LINE__,__FUNCTION__,#condition); \
                fflush(fp_log); \
                exit(-1); \
        }\
}while(0)


#define E3_LOG(format,...) {\
	fprintf(fp_log,"%s [log] %s:%d %s() ",current_time(),__FILE__,__LINE__,__FUNCTION__); \
	fprintf(fp_log,(format),##__VA_ARGS__);} \
	fflush(fp_log);

#define E3_WARN(format,...) {\
	fprintf(fp_log,"%s [warn] %s:%d %s() ",current_time(),__FILE__,__LINE__,__FUNCTION__); \
	fprintf(fp_log,(format),##__VA_ARGS__);} \
	fflush(fp_log);

#define E3_ERROR(format,...) {\
	fprintf(fp_log,"%s [error] %s:%d %s() ",current_time(),__FILE__,__LINE__,__FUNCTION__); \
	fprintf(fp_log,(format),##__VA_ARGS__);} \
	fflush(fp_log);

extern FILE * fp_log;
extern time_t log_time;

#if defined(USE_STD_LOG)
	#define LOG_FILE_PATH "/dev/stdout"
#else
	#define LOG_FILE_PATH "/var/log/e3vswitch.log"
#endif
#endif
