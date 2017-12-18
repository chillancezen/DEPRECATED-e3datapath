/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <rte_eal.h>
#include <e3test.h>
#include <stdio.h>
#include <unistd.h>
#include <e3infra/include/lcore-extension.h>
#include <e3infra/include/e3-ini-config.h>
#include <e3infra/include/e3-log.h>
#include <e3net/include/e3iface-inventory.h>

DECLARE_TEST_CASE(tc_main);

Suite * suite_total=NULL;
void init_suite(void)
{
	suite_total=suite_create("e3datapath_suite");
	E3_ASSERT(suite_total);
}
E3_init(init_suite,E3_TASK_PRI_TEST_SUITE);


void global_test_setup(void)
{
	int  pport;
	char * pbp_pci_addr;
	char * csp_pci_addr;
	char * cbp_pci_addr=get_ini_option_string("test","cbp_pci_addr");
	if(cbp_pci_addr)
		create_e3iface(0,
			cbp_pci_addr,
			E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,
			E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,
			&pport);
	csp_pci_addr=get_ini_option_string("test","csp_pci_addr");
	if(csp_pci_addr)
		create_e3iface(0,
			csp_pci_addr,
			E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,
			E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,
			&pport);
	pbp_pci_addr=get_ini_option_string("test","pbp_pci_addr");
	if(pbp_pci_addr)
		create_e3iface(0,
			pbp_pci_addr,
			E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,
			E3IFACE_ROLE_PROVIDER_BACKBONE_PORT,
			&pport);
}
int main(int argc, char **argv)
{	
    int n;
	SRunner *sr;
	pthread_t pid;
	rte_eal_init(argc, argv);
	init_registered_tasks();
	global_test_setup();
	sr = srunner_create(suite_total);
	srunner_run_all(sr, CK_ENV);
	n = srunner_ntests_failed(sr);
	srunner_free(sr);

	
	return 0;
}
