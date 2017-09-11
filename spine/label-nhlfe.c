#include <label-nhlfe.h>
#include <e3_init.h>
#include <e3_log.h>
#include <rte_malloc.h>
#include <string.h>

struct topological_neighbor * topological_neighbor_base;
struct next_hop             * next_hop_base;

void label_nhlfe_init(void)
{
	topological_neighbor_base=rte_zmalloc(NULL,
			sizeof(struct topological_neighbor)*MAX_TOPOLOGICAL_NEIGHBOURS,
			64);
	E3_ASSERT(topological_neighbor_base);
	next_hop_base=rte_zmalloc(NULL,
			sizeof(struct next_hop)*MAX_NEXT_HOPS,
			64);
	E3_ASSERT(next_hop_base);
	
}
E3_init(label_nhlfe_init, TASK_PRIORITY_RESOURCE_INIT);

/*
*input:le_ip_as_key, the ip in little endian as the key to index
*     :mac, the mac address bound with ip.
*return: any negative integer returned indicates failure,otherwise
*the allocated index is returned.
*/
int register_topological_neighbour(uint32_t le_ip_as_key,
	uint8_t *mac)
{
	int idx=0;
	int target_index=0;
	for(;idx<MAX_TOPOLOGICAL_NEIGHBOURS;idx++)
		if(topological_neighbor_base[idx].is_valid&&
			topological_neighbor_base[idx].neighbour_ip_as_le==le_ip_as_key)
			return -1;
		
	for(;target_index<MAX_TOPOLOGICAL_NEIGHBOURS;target_index++)
		if(!topological_neighbor_base[target_index].is_valid)
			break;
	if(target_index>=MAX_TOPOLOGICAL_NEIGHBOURS)
		return -2;

	topological_neighbor_base[target_index].is_valid=1;
	topological_neighbor_base[target_index].neighbour_ip_as_le=le_ip_as_key;
	memcpy(topological_neighbor_base[target_index].mac,mac,6);
	
	return target_index;
}

/*
*find the corresponding neighbour index according to ip
*if found, the index(>=0) is returned,otherwise, a negative 
*value is returned.
*/
int search_topological_neighbour(uint32_t le_ip_as_key)
{
	int idx=0;
	for(idx=0;idx<MAX_TOPOLOGICAL_NEIGHBOURS;idx++)
		if(topological_neighbor_base[idx].is_valid&&
			topological_neighbor_base[idx].neighbour_ip_as_le==le_ip_as_key)
			return idx;
	return -1;
}

void label_nhlfe_module_test(void)
{
	printf("%d\n",register_topological_neighbour(0x122,"dsdssds"));
	printf("%d\n",register_topological_neighbour(0x1221,"dsdssds"));
	//topological_neighbour_at(1)->is_valid=0;
	printf("%d\n",register_topological_neighbour(0x123,"dsdssds"));

	printf("found:%d\n",search_topological_neighbour(0x123));
	printf("found:%d\n",search_topological_neighbour(0x1213));
	//topological_neighbour_at(2)->is_valid=1;
	//printf("%p\n",topological_neighbour_is_valid(topological_neighbour_at(0)));
	//printf("%p\n",topological_neighbour_is_valid(topological_neighbour_at(1)));
	//printf("%p\n",topological_neighbour_is_valid(topological_neighbour_at(2)));
	
	//printf("%p\n",topological_neighbour_at(-232));
	//printf("%p\n",topological_neighbour_at(1));
	//printf("%p\n",topological_neighbour_at(MAX_TOPOLOGICAL_NEIGHBOURS-1));
	//printf("%p\n",next_hop_at(MAX_TOPOLOGICAL_NEIGHBOURS));
	//printf("%p\n",next_hop_at(MAX_NEXT_HOPS-1));
	//printf("%p\n",next_hop_at(MAX_NEXT_HOPS));
	//printf("%d\n",sizeof(struct topological_neighbor));
	//printf("%d\n",sizeof(struct next_hop));
	
}