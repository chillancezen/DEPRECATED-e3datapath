#ifndef _LABEL_NHLFE_H
#define _LABEL_NHLFE_H
#include <stdint.h>

/*this two definition corresponds to the bit width of label_entry.NHLFE
*and may be adjusted later
*/
#define MAX_TOPOLOGICAL_NEIGHBOURS 1024
#define MAX_NEXT_HOPS 1024
#define MAX_MULTICAST_NEXT_HOPS 1024


/*topological_neighbour is system-wide resource,
*in order for neighbours to be clearly understood,
*non-overlapped address space is strongly recommeded
*/
struct topological_neighbor{
	uint32_t neighbour_ip_as_le; //simply,ip as key
	uint8_t  mac[6];
	uint8_t is_valid;
}__attribute__((aligned(4)));

/*next_hop maps to link_pair in control plane abstraction,
*and is structured to store local e3interface index and next hop's neighbour
*by searching with next_hop,we can determine the L2 src&dst address
*/

struct next_hop{
	int local_e3iface_index; //both fields as the key
	int remote_neighbor_index;
	uint8_t is_valid;
}__attribute__((aligned(4)));


#define MAX_HOPS_IN_MULTICAST_GROUP 64 /* 64 forks in a tree in the topology
										is supposed to be enough*/

struct multicast_next_hops{
	uint64_t multicast_group_id;/*usually uniquely mapped to E-SERVICE
								 in whatever manner*/
	uint8_t  is_valid;
	uint8_t  nr_hops;
	uint16_t next_hops[MAX_HOPS_IN_MULTICAST_GROUP];
}__attribute__((aligned(4)));



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
