#ifndef _E3_CONFIG
#define _E3_CONFIG
/*the common options will be put here as DPDK's rte_config.h does*/
#define MAX_NUMBER_OF_E3INTERFACE 512 /*the maximum number of the registerd e3_interfaces,
										constrained by RTE_MAX_ETH_PORTS(default:256)*/

#define DEFAULT_RX_DESCRIPTOR 512 
#define DEFAULT_TX_DESCRIPTOR 512 


//#define ENABLE_MULTIPLE_API_ENDPOINT 

#define MAJOR_VERSION_NUMBER 0x1
#define MINOR_VERSION_NUMBER 0x0
#define BUILD_VERSION_NUMBER 0x1

#endif