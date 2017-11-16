/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <rte_eal.h>
#include <e3test.h>
#include <stdio.h>
#include <e3infra/include/lcore_extension.h>
DECLARE_TEST_CASE(tc_main);

Suite * suite_total=NULL;
void init_suite(void)
{
	suite_total=suite_create("e3datapath_suite");
	E3_ASSERT(suite_total);
}
E3_init(init_suite,E3_TASK_PRI_TEST_SUITE);

int main(int argc, char **argv)
{
    int lcore_id;
    int n;
    SRunner *sr;
	rte_eal_init(argc, argv);
	init_registered_tasks();
    sr = srunner_create(suite_total);
    srunner_run_all(sr, CK_ENV);
    n = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (n == 0) ? EXIT_SUCCESS : EXIT_FAILURE;	
}
