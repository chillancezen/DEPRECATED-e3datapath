/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _LEAF_E_SERVICE_H
#define _LEAF_E_SERVICE_H
#include <stdint.h>
#include <e3net/include/common-nhlfe.h>
#include <e3infra/include/fast-index.h>
#include <urcu-qsbr.h>
#include <rte_spinlock.h>
#include <rte_rwlock.h>

#define MAX_E_LINE_SERVICES 4096
#define MAX_E_LAN_SERVICES 4096


struct ether_e_line{
	/*
	*if label_to_push and NHLFE is set,is_cbp_ready will be set to 1
	*if e3iface and vlan_tci is set,is_csp_ready is set to 1
	*/
	uint16_t  is_cbp_ready:1;
	uint16_t  is_csp_ready:1;
	uint16_t  is_valid:1;
	
	uint16_t index;
	/*
	*bacbone side fields
	*/
	uint32_t label_to_push;
	uint16_t NHLFE;
	/*
	*customer side fields
	*/
	uint16_t e3iface;
	uint16_t vlan_tci;
	
	uint16_t ref_cnt;
}__attribute__((packed));



#define MAX_PORTS_IN_E_LAN_SERVICE 128
#define MAX_NHLFE_IN_E_LAN_SERVICE 128

struct ether_e_lan{
	uint8_t is_valid;
	uint8_t is_releasing;
	uint16_t index;
	uint16_t ref_cnt;
	uint16_t nr_ports;
	uint16_t nr_nhlfes;
	uint16_t multicast_NHLFE;
	uint32_t multicast_label;
	struct findex_2_4_base * fib_base;
	struct{
		uint16_t is_valid;
		uint16_t iface;
		uint16_t vlan_tci;
		uint16_t reserved0;
	}ports[MAX_PORTS_IN_E_LAN_SERVICE];
	struct{
		uint16_t is_valid;
		uint16_t NHLFE;
		uint32_t label_to_push;
	}nhlfes[MAX_NHLFE_IN_E_LAN_SERVICE];
	struct rcu_head rcu;
}__attribute__((packed));

extern struct ether_e_line * e_line_base;
#define _find_e_line_service(index) ((((index)>=0)&&((index)<MAX_E_LINE_SERVICES))?&e_line_base[(index)]:NULL)
#define find_e_line_service(index) (((((index)>=0)&&((index)<MAX_E_LINE_SERVICES))&&(e_line_base[(index)].is_valid))?&e_line_base[(index)]:NULL)
int register_e_line_service(void);
int register_e_line_port(int eline_index,int e3iface,int vlan_tci);
int register_e_line_nhlfe(int eline_index,int NHLFE,int label_to_push);
int delete_e_line_port(int eline_index);

int reference_e_line_service(int index);
int dereference_e_line_service(int index);
int delete_e_line_service(int index);

extern struct ether_e_lan  * e_lan_base;
#define _find_e_lan_service(index) ((((index)>=0)&&((index)<MAX_E_LAN_SERVICES))?&e_lan_base[(index)]:NULL)
#define find_e_lan_service(index) (((((index)>=0)&&((index)<MAX_E_LAN_SERVICES))&&(e_lan_base[(index)].is_valid))?&e_lan_base[(index)]:NULL)
int register_e_lan_service(void);
int reference_e_lan_service(int index);
int dereference_e_lan_service(int index);
int delete_e_lan_service(int index);
int register_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci);
int find_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci);
int find_e_lan_port_locked(int elan_index,uint16_t e3iface,uint16_t vlan_tci);
int delete_e_lan_port(int elan_index,int port_index);
int register_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push);
int find_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push);
int find_e_lan_nhlfe_locked(int elan_index,uint16_t nhlfe,uint32_t label_to_push);
int delete_e_lan_nhlfe(int elan_index,int nhlfe_index);

struct e_lan_fwd_entry{
	union{
		struct{
			uint16_t is_port_entry;
			union{
				uint16_t field_u16;
				uint16_t e3iface;
				uint16_t NHLFE;
			};
			union{
				uint32_t field_u32;
				uint32_t vlan_tci;
				uint32_t label_to_push;
			};
		};
		uint64_t entry_as_u64;
	};
}__attribute__((packed));

#define mac_to_findex_2_4_key(mac,key) {\
	(key)->key_index=MAKE_UINT16((mac)[3],(mac)[4]); \
	(key)->tag1=MAKE_UINT16((mac)[0],(mac)[1]); \
	(key)->tag2=MAKE_UINT16((mac)[2],(mac)[5]); \
}

#define findex_2_4_key_to_mac(key,mac) {\
    (mac)[0]=HIGH_UINT16((key)->tag1); \
    (mac)[1]=LOW_UINT16((key)->tag1); \
    (mac)[2]=HIGH_UINT16((key)->tag2); \
    (mac)[3]=HIGH_UINT16((key)->key_index); \
    (mac)[4]=LOW_UINT16((key)->key_index); \
    (mac)[5]=LOW_UINT16((key)->tag2); \
}
int register_e_lan_fwd_entry(int elan_index,uint8_t * mac,struct e_lan_fwd_entry * fwd_entry);
int delete_e_lan_fwd_entry(int elan_index,uint8_t *mac);

#endif