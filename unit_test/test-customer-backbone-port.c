#include <e3test.h>
#include <leaf/include/customer-backbone-port.h>
#include <e3infra/include/e3-ini-config.h>
#include <e3infra/include/e3-hashmap.h>
DECLARE_TEST_CASE(tc_customer_backbone_port);

START_TEST(cbp_generic){
	/*
	*environmental pre-setup
	*/
	char * cbp_pci_addr=get_ini_option_string("test","cbp_pci_addr");
	printf("pci:%s\n",cbp_pci_addr);
	/*
	*environmental cleanup
	*/
}
END_TEST
ADD_TEST(cbp_generic);
