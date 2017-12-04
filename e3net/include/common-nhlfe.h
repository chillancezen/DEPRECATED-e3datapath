/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _COMMON_NHLFE_H
#define _COMMON_NHLFE_H
#include <stdint.h>

struct common_neighbor{
	uint32_t neighbour_ip_as_le;
	uint16_t ref_cnt;
	uint16_t index;
	uint8_t  mac[6];
	uint8_t  is_valid;
	uint8_t  reserved0;
}__attribute__((packed));
/*
Python definition:
size of neighbor: 16
<Field type=c_ubyte_Array_4, ofs=0, size=4> neighbor.neighbour_ip_as_le
<Field type=c_short, ofs=4, size=2> neighbor.ref_cnt
<Field type=c_short, ofs=6, size=2> neighbor.index
<Field type=c_ubyte_Array_6, ofs=8, size=6> neighbor.mac
<Field type=c_ubyte, ofs=14, size=1> neighbor.is_valid
<Field type=c_ubyte, ofs=15, size=1> neighbor.reserved0

*/
struct common_nexthop{
	uint16_t local_e3iface;
	uint16_t common_neighbor_index;
	uint16_t ref_cnt;
	uint16_t index;
	uint8_t  is_valid;
	uint8_t  reserved0;
	uint16_t reserved1;
}__attribute__((packed));
/*
size of nexthop: 12
<Field type=c_short, ofs=0, size=2> local_e3iface
<Field type=c_short, ofs=2, size=2> common_neighbor_index
<Field type=c_short, ofs=4, size=2> ref_cnt
<Field type=c_short, ofs=6, size=2> index
<Field type=c_byte, ofs=8, size=1> is_valid
<Field type=c_ubyte, ofs=9, size=1> reserved0
<Field type=c_ushort, ofs=10, size=2> reserved1

*/
#define MAX_COMMON_NEIGHBORS 2048
#define MAX_COMMON_NEXTHOPS  2048

extern struct common_neighbor * neighbor_base;
int register_common_neighbor(struct common_neighbor * neighbor);
int refresh_common_neighbor_mac(struct common_neighbor * neighbor);
#define _find_common_neighbor(index) ((((index)>=0)&&((index)<MAX_COMMON_NEIGHBORS))?&neighbor_base[(index)]:NULL)
#define find_common_neighbor(index) (((((index)>=0)&&((index)<MAX_COMMON_NEIGHBORS))&(neighbor_base[(index)].is_valid))?&neighbor_base[(index)]:NULL)
int reference_common_nrighbor(int index);
int dereference_common_neighbor(int index);
int delete_common_neighbor(int index);


extern struct common_nexthop  * nexthop_base;
int register_common_nexthop(struct common_nexthop * hop);
#define _find_common_nexthop(index) ((((index)>=0)&&((index)<MAX_COMMON_NEXTHOPS))?&nexthop_base[(index)]:NULL)
#define find_common_nexthop(index) (((((index)>=0)&&((index)<MAX_COMMON_NEXTHOPS))&&(nexthop_base[(index)].is_valid))?&nexthop_base[(index)]:NULL)
int reference_common_nexthop(int index);
int dereference_common_nexthop(int index);
int delete_common_nexthop(int index);

void __read_lock_neighbor(void);
void __read_unlock_neighbor(void);
void __read_lock_nexthop(void);
void __read_unlock_nexthop(void);

#endif
