#ifndef _LABEL_NHLFE_H
#define _LABEL_NHLFE_H
#include <stdint.h>

/*this two definition corresponds to the bit width of label_entry.NHLFE
*and may be adjusted later
*/
#define MAX_TOPOLOGICAL_NEIGHBOURS 1024
#define MAX_NEXT_HOPS 1024
#define MAX_MULTICAST_NEXT_HOPS 2048/*max E-LAN service supported*/


/*topological_neighbour is system-wide resource,
*in order for neighbours to be clearly understood,
*non-overlapped address space is strongly recommeded
*/
struct topological_neighbor{
	uint32_t neighbour_ip_as_le; //simply,ip as key
	uint8_t  mac[6];
	uint8_t is_valid;
	uint8_t reserved0;
}__attribute__((packed));
/*
C definition:
neighbour_ip_as_le (offset:  0 size:  4 prev_gap:0)
			   mac (offset:  4 size:  6 prev_gap:0)
		  is_valid (offset: 10 size:  1 prev_gap:0)
		 reserved0 (offset: 11 size:  1 prev_gap:0)
		 
Python definition:
	<Field type=c_ubyte_Array_4, ofs=0, size=4>
	<Field type=c_ubyte_Array_6, ofs=4, size=6>
	<Field type=c_ubyte, ofs=10, size=1>
	<Field type=c_ubyte, ofs=11, size=1>

*/
/*next_hop maps to link_pair in control plane abstraction,
*and is structured to store local e3interface index and next hop's neighbour
*by searching with next_hop,we can determine the L2 src&dst address
*/

struct next_hop{
	int local_e3iface_index; //both fields as the key
	int remote_neighbor_index;
	uint8_t is_valid;
	uint8_t reserved0;
}__attribute__((packed));
/*
C definition:
 local_e3iface_index  (offset:  0 size:  4 prev_gap:0)
remote_neighbor_index (offset:  4 size:  4 prev_gap:0)
            is_valid  (offset:  8 size:  1 prev_gap:0)
           reserved0  (offset:  9 size:  1 prev_gap:0)
           
Python Definition:
	<Field type=c_uint, ofs=0, size=4>
	<Field type=c_uint, ofs=4, size=4>
	<Field type=c_ubyte, ofs=8, size=1>
	<Field type=c_ubyte, ofs=9, size=1>

*/

#define MAX_HOPS_IN_MULTICAST_GROUP 64 /* 64 forks in a tree in the topology
										is supposed to be enough*/

struct multicast_next_hops{
	uint64_t multicast_group_id;/*usually uniquely mapped to E-SERVICE
								 in whatever manner,
								 2017-10.14:this field will be reserved for other purposes*/
	uint8_t  is_valid;
	uint8_t  nr_hops;
	uint16_t next_hops[MAX_HOPS_IN_MULTICAST_GROUP];
	uint32_t next_hops_labels[MAX_HOPS_IN_MULTICAST_GROUP];
}__attribute__((packed));
/*
C definition:

multicast_group_id (offset:  0 size:  8 prev_gap:0)
		   is_valid (offset:  8 size:  1 prev_gap:0)
			nr_hops (offset:  9 size:  1 prev_gap:0)
		  next_hops (offset: 10 size:128 prev_gap:0)
   next_hops_labels (offset:138 size:256 prev_gap:0)
   
Python definition:
<Field type=c_ulong, ofs=0, size=8>
<Field type=c_ubyte, ofs=8, size=1>
<Field type=c_ubyte, ofs=9, size=1>
<Field type=c_ushort_Array_64, ofs=10, size=128>
<Field type=c_uint_Array_64, ofs=138, size=256>
*/


extern struct topological_neighbor * topological_neighbor_base;
extern struct next_hop             * next_hop_base;
extern struct multicast_next_hops  * mnext_hops_base;
#define topological_neighbour_at(index) ((((index)>=0)&&((index)<MAX_TOPOLOGICAL_NEIGHBOURS))?\
										&topological_neighbor_base[(index)]: \
										NULL)
#define topological_neighbour_is_valid(neigh) (!!(neigh)->is_valid)
int register_topological_neighbour(uint32_t le_ip_as_key,uint8_t *mac);
int search_topological_neighbour(uint32_t le_ip_as_key);
int update_neighbour_mac(uint32_t le_ip_as_key,
		uint8_t *mac);




int register_next_hop(int local_e3iface,int remote_neighbour);
int search_next_hop(int local_e3iface,int remote_neighbour);
#define next_hop_at(index) ((((index)>=0)&&((index)<MAX_NEXT_HOPS))?\
										&next_hop_base[(index)]: \
										NULL)					
#define next_hop_is_valid(nhop) (!!(nhop)->is_valid)


int register_multicast_next_hops(struct multicast_next_hops * mnh);
int search_multicast_next_hopss(struct multicast_next_hops * mnh);
#define mnext_hops_at(index) ((((index)>=0)&&((index)<MAX_MULTICAST_NEXT_HOPS))?\
										&mnext_hops_base[(index)]: \
										NULL)
#define mnext_hops_is_valid(mnh)  (!!(mnh)->is_valid)




#endif
