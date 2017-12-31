/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3infra/include/e3-log.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3infra/include/e3-init.h>
#include <stdio.h>
#include <time.h>
#include <rte_mempool.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_atomic.h>
#include <e3infra/include/malloc-wrapper.h>
FILE * fp_live_debug=NULL;

rte_atomic32_t cnt_alloc=RTE_ATOMIC32_INIT(0);
rte_atomic32_t cnt_free=RTE_ATOMIC32_INIT(0);


void * RTE_ZMALLOC(const char *type, size_t size, unsigned align)
{
	void * ret=rte_zmalloc(type,size,align);
	if(ret){
		rte_atomic32_inc(&cnt_alloc);
	}
	return ret;
}
void * 	RTE_ZMALLOC_SOCKET(const char *type, size_t size, unsigned align, int socket)
{
	void * ret=rte_zmalloc_socket(type,size,align,socket);
	if(ret){
		rte_atomic32_inc(&cnt_alloc);
	}
	return ret;
}
void RTE_FREE(void *ptr)
{
	if(ptr){
		rte_atomic32_inc(&cnt_free);
	}
	rte_free(ptr);
}

void live_debug_init(void)
{
	fp_live_debug=fopen(DEBUG_FILE_PATH,"a");
	E3_ASSERT(fp_live_debug);
	E3_LOG("live debug subroutine initialized\n");
}
E3_init(live_debug_init,TASK_PTIORITY_LOW);

e3_type dump_memory_structure(e3_type api_service)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char s[64];
	strftime(s, sizeof(s), "%c", tm);
	fprintf(fp_live_debug,"-------------------dump memory at %s\n",s);
	#if 0
		/*1.dump memory segments,this is actual the physical memory segment layout*/
		rte_dump_physmem_layout(fp_live_debug);
		/*2.dump memory zones, which is the free segments*/
		rte_memzone_dump(fp_live_debug);
		/*3.dump all the mempool object*/
		rte_mempool_list_dump(fp_live_debug);
		/*4.dump all the allocated memory object from DPDK hugepages*/
		rte_malloc_dump_stats(fp_live_debug,NULL);
	#else
		fprintf(fp_live_debug,"allocation   counter:%d\n",cnt_alloc.cnt);
		fprintf(fp_live_debug,"deallocation counter:%d\n",cnt_free.cnt);
	#endif
	fflush(fp_live_debug);
	return 0;
}
DECLARE_E3_API(live_debug)={
	.api_name="dump_memory_structure",
	.api_desc="dump the memory relevant objects",
	.api_callback_func=(api_callback_func)dump_memory_structure,
	.args_desc={
		{.type=e3_arg_type_none,},
	},	
};