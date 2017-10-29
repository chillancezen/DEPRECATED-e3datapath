#ifndef _LEAF_E_SERVICE_H
#define _LEAF_E_SERVICE_H
#include <stdint.h>
#include <e3net/include/common-nhlfe.h>
#include <fast-index.h>
#include <urcu-qsbr.h>

#define MAX_E_LINE_SERVICES 4096
#define MAX_E_LAN_SERVICES 4096


struct ether_e_line{
	uint16_t is_valid;
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


struct ether_e_lan{
	uint16_t is_valid;
	uint16_t index;
	uint16_t ref_cnt;
	uint16_t multicast_NHLFE;
	uint32_t multicast_label;
	struct findex_2_1_6_base * fib_base;
	struct rcu_head rcu;
}__attribute__((packed));

extern struct ether_e_line * e_line_base;
#define find_e_line_service(index) ((((index)>=0)&&((index)<MAX_E_LINE_SERVICES))?&e_line_base[(index)]:NULL)
int register_e_line_service(struct ether_e_line * eline);
int reference_e_line_service(int index);
int dereference_e_line_service(int index);
int delete_e_line_service(int index);

extern struct ether_e_lan  * e_lan_base;

#endif