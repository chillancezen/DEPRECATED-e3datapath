#ifndef _CUSTOMER_SERVICE_PORT_H
#define _CUSTOMER_SERVICE_PORT_H
#include <stdint.h>

struct csp_private{
	uint32_t attached;/*whether attached to an ethernet service*/
	uint32_t e_service;
	uint32_t service_index;
	uint32_t vlan_tci;
};
#endif