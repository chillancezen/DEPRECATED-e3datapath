/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _E3_LOG_H
#define _E3_LOG_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <e3infra/include/e3_log_feature.h>


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
				log_fatal("%s",#condition); \
                exit(-1); \
        }\
}while(0)


#define E3_LOG(format,...) {\
	log_info((format),##__VA_ARGS__); \
}


#define E3_WARN(format,...) {\
	log_warn((format),##__VA_ARGS__); \
}

#define E3_ERROR(format,...) {\
	log_error((format),##__VA_ARGS__); \
}

/*
*E3_DEBUG is special because we usually put it in fastpath
*so it should not appear in RELEASE target
*/
#if BUILD_TYPE==BUILD_TYPE_DEBUG
	#define E3_DEBUG(format,...){\
		log_debug((format),##__VA_ARGS__); \
	}
#else
	#define E3_DEBUG(format,...)
#endif

extern time_t log_time;

#endif
