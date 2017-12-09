/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _LABEL_NHLFE_H
#define _LABEL_NHLFE_H
#include <stdint.h>
/*
*max E-LAN service supported
*that's to say,a single node can only service MAX_MULTICAST_NEXT_HOPS 
*E-LAN services
*/
#define MAX_MULTICAST_NEXT_HOPS 2048

#define MAX_HOPS_IN_MULTICAST_GROUP 64 /* 64 forks in a tree in the topology
										is supposed to be enough*/

struct multicast_next_hops{
	uint16_t  is_valid;
	uint16_t  nr_hops;
	uint16_t index;
	uint16_t ref_cnt;
	struct{
		uint16_t is_valid;
		uint16_t next_hop;
		uint32_t label_to_push;
	}nexthops[MAX_HOPS_IN_MULTICAST_GROUP];
}__attribute__((packed));

/*
Python Definition:
size of multicast_next_hops: 520
<Field type=c_ushort, ofs=0, size=2> is_valid
<Field type=c_short, ofs=2, size=2> nr_hops
<Field type=c_short, ofs=4, size=2> index
<Field type=c_short, ofs=6, size=2> ref_cnt
<Field type=multicast_next_hop_entry_Array_64, ofs=8, size=512> nexthops
*/

extern struct multicast_next_hops  * mnext_hops_base;

#define _find_mnext_hops(index) ((((index)>=0)&&((index)<MAX_MULTICAST_NEXT_HOPS))?\
										&mnext_hops_base[(index)]: \
										NULL)
#define find_mnext_hops(index) (((((index)>=0)&&((index)<MAX_MULTICAST_NEXT_HOPS))&&(mnext_hops_base[(index)].is_valid))?\
										&mnext_hops_base[(index)]: \
										NULL)

int register_multicast_nexthops();
#define mnext_hops_is_valid(mnh)  (!!(mnh)->is_valid)
void __read_lock_mnexthop(void);
void __read_unlock_mnexthop(void);


#endif
