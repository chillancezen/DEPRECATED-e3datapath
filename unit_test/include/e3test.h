/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _E3TEST_H
#define _E3TEST_H
#include <check.h>
#include <e3_init.h>
#include <e3_log.h>
#define E3_TASK_PRI_TEST_SUITE   (0x100+TASK_PTIORITY_LOW)
#define E3_TASK_PRI_TEST_CASE    (0x1000+TASK_PTIORITY_LOW)
#define E3_TASK_PRI_TEST_ELEMENT (0x10000+TASK_PTIORITY_LOW)
extern Suite * suite_total;

#define DECLARE_TEST_CASE(tc_name) \
	static char  * _local_tc_name=#tc_name; \
	static TCase * _local_tc_case=NULL; \
	static void create_tc_##tc_name(void){ \
		_local_tc_case=tcase_create(#tc_name); \
		E3_ASSERT(_local_tc_case); \
		suite_add_tcase(suite_total,_local_tc_case); \
	} \
	E3_init(create_tc_##tc_name,E3_TASK_PRI_TEST_CASE)

#define ADD_TEST(Tfunc) \
	static void ____##Tfunc(void){ \
		tcase_add_test(_local_tc_case,Tfunc); \
	} \
	E3_init(____##Tfunc,E3_TASK_PRI_TEST_ELEMENT)
#endif
