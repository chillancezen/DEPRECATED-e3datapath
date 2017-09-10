#ifndef _LABEL_NHLFE_H
#define _LABEL_NHLFE_H
#include <stdint.h>

#define MAX_TOPOLOGICAL_NEIGHBOURS 4096
#define MAX_NEXT_HOPS (MAX_TOPOLOGICAL_NEIGHBOURS*2)

/*topological_neighbour is system-wide resource,
*in order for neighbours to be clearly understood,
*non-overlapped address space is strongly recommeded
*/
struct topological_neighbor{
	uint32_t neighbour_ip_as_le;
	uint8_t  mac[6];
	uint8_t is_valid;
}__attribute__((aligned(4)));

/*next_hop maps to link_pair in control plane abstraction,
*and is structured to store local e3interface index and next hop's neighbour
*by searching with next_hop,we can determine the L2 src&dst address
*/

struct next_hop{
	int local_e3iface_index;
	int remote_neighbor_index;
	uint8_t is_valid;
}__attribute__((aligned(4)));

#endif