#include <label-nhlfe.h>
#include <e3_init.h>
#include <e3_log.h>
#include <rte_malloc.h>
#include <string.h>
#include <stdlib.h>
#include <e3-api-wrapper.h>
#include <util.h>
struct topological_neighbor * topological_neighbor_base;
struct next_hop             * next_hop_base;
struct multicast_next_hops  * mnext_hops_base;
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
	mnext_hops_base=rte_zmalloc(NULL,
			sizeof(struct multicast_next_hops)*MAX_MULTICAST_NEXT_HOPS,
			64);
	E3_ASSERT(mnext_hops_base);
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
int update_neighbour_mac(uint32_t le_ip_as_key,
		uint8_t *mac)
{
	struct topological_neighbor * pneighbour;
	int neighbor=search_topological_neighbour(le_ip_as_key);
	if(neighbor<0)
		return -1;
	pneighbour=topological_neighbour_at(neighbor);
	memcpy(pneighbour->mac,mac,6);
	return 0;
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
/*if success, the real index is returned,otherwise,
a negative integer is returned*/
int register_next_hop(int local_e3iface,int remote_neighbour)
{
	int idx=0;
	for(idx=0;idx<MAX_NEXT_HOPS;idx++)
		if(next_hop_base[idx].is_valid&&
			(next_hop_base[idx].local_e3iface_index==local_e3iface)&&
			(next_hop_base[idx].remote_neighbor_index==remote_neighbour))
			return -1;
	for(idx=0;idx<MAX_NEXT_HOPS;idx++)
		if(!next_hop_base[idx].is_valid)
			break;
	if(idx>=MAX_NEXT_HOPS)
		return -2;
	next_hop_base[idx].remote_neighbor_index=remote_neighbour;
	next_hop_base[idx].local_e3iface_index=local_e3iface;
	next_hop_base[idx].is_valid=1;
	return idx;
}

int search_next_hop(int local_e3iface,int remote_neighbour)
{
	int idx=0;
	for(idx=0;idx<MAX_NEXT_HOPS;idx++)
		if(next_hop_base[idx].is_valid&&
			(next_hop_base[idx].local_e3iface_index==local_e3iface)&&
			(next_hop_base[idx].remote_neighbor_index==remote_neighbour))
			return idx;
	return -1;
}

int _hop_index_comare(const void * arg1, const void * arg2)
{
	return (*(uint16_t*)arg1)-(*(uint16_t*)arg2);
}
void _rerrange_multicast_next_hop_indexs(struct multicast_next_hops * mnh)
{
	qsort(mnh->next_hops,mnh->nr_hops,sizeof(uint16_t),_hop_index_comare);
}
/*both structures are well sorted
*if equal,0 is return. otherwise non-zero is instead.
*/
int _compare_multicast_next_hop_indexs(const struct multicast_next_hops * hops1,
										const struct multicast_next_hops * hops2)
{
	if(hops1->nr_hops!=hops2->nr_hops)
		return -1;
	int idx=0;
	for(idx=0;idx<hops1->nr_hops;idx++)
		if(hops1->next_hops[idx]!=hops2->next_hops[idx])
			return -2;
	return 0;
}
int register_multicast_next_hops(struct multicast_next_hops * mnh)
{
	int idx=0;
	int iptr;
	_rerrange_multicast_next_hop_indexs(mnh);
	for(idx=0;idx<MAX_MULTICAST_NEXT_HOPS;idx++)
		if(mnext_hops_base[idx].is_valid&&
			(!_compare_multicast_next_hop_indexs(mnh,&mnext_hops_base[idx])))
			return -1;
	for(idx=0;idx<MAX_MULTICAST_NEXT_HOPS;idx++)
		if(!mnext_hops_base[idx].is_valid)
			break;
	if(idx>=MAX_MULTICAST_NEXT_HOPS)
		return -2;
	mnext_hops_base[idx].multicast_group_id=mnh->multicast_group_id;
	mnext_hops_base[idx].nr_hops=mnh->nr_hops;
	for(iptr=0;iptr<mnext_hops_base[idx].nr_hops;iptr++)
		mnext_hops_base[idx].next_hops[iptr]=mnh->next_hops[iptr];
	mnext_hops_base[idx].is_valid=1;
	return idx;
}
int search_multicast_next_hopss(struct multicast_next_hops * mnh)
{
	int idx=0;
	for(idx=0;idx<MAX_MULTICAST_NEXT_HOPS;idx++)
		if(mnext_hops_base[idx].is_valid&&
			(!_compare_multicast_next_hop_indexs(&mnext_hops_base[idx],mnh)))
			return idx;
	return -1;
}
void dump_topological_neighbour_definition(void)
{
	int last_offset=0,last_size=0;
	puts("dump definition: struct topological_neighbor");
	dump_field(struct topological_neighbor,neighbour_ip_as_le);
	dump_field(struct topological_neighbor,mac);
	dump_field(struct topological_neighbor,is_valid);
	dump_field(struct topological_neighbor,reserved0);
	
}
void label_nhlfe_module_test(void)
{
	/*
	int idx=0;
	for(;idx<1478;idx++)
		register_topological_neighbour(0x122f34+idx,"dsdssds");
	//topological_neighbor_base[1022].is_valid=0;
	//dump_topological_neighbour_definition();
	
	struct multicast_next_hops hops={
		.next_hops={1,2,34,2,544,3,432,234},
		.nr_hops=8,
	};
	printf("mnext:%d\n",register_multicast_next_hops(&hops));
	hops.next_hops[0]=0x24;
	printf("mnext:%d\n",search_multicast_next_hopss(&hops));
	
	rerrange_multicast_next_hop_indexs(&hops);
	int idx=0;
	for(idx=0;idx<hops.nr_hops;idx++)
		printf("hop:%d\n",hops.next_hops[idx]);
	*/
		
	
	//printf("size:%d\n",sizeof(struct multicast_next_hops));
	//printf("next hop:%d\n",register_next_hop(2,23));
	//printf("next hop:%d\n",register_next_hop(2,23));
	//printf("next hop:%d\n",register_next_hop(2,223));

	//printf("found hop:%d\n",search_next_hop(2,223));
	//printf("found hop:%d\n",search_next_hop(2,224));
	//printf("%d\n",register_topological_neighbour(0x122,"dsdssds"));
	//printf("%d\n",register_topological_neighbour(0x1221,"dsdssds"));
	//topological_neighbour_at(1)->is_valid=0;
	//printf("%d\n",register_topological_neighbour(0x123,"dsdssds"));

	//printf("found:%d\n",search_topological_neighbour(0x123));
	//printf("found:%d\n",search_topological_neighbour(0x1213));
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

/*export C&Python APIs*/

/*register a neighbor
*ip_string:     xxx.xxx.xxx.xxx (4x4)
*mac_string:    xx:xx:xx:xx:xx:xx (3x6)
*/
uint32_t _ip_string_to_u32_le(const char * ip_string)
{
	union{
		uint32_t ip_as_u32_le;
		uint8_t  ip_as_u8a[4];
	}ip_stub;
	int _ip_tmp[4];
	sscanf(ip_string,"%d.%d.%d.%d",&_ip_tmp[3],
					&_ip_tmp[2],
					&_ip_tmp[1],
					&_ip_tmp[0]);
	ip_stub.ip_as_u8a[0]=_ip_tmp[0];
	ip_stub.ip_as_u8a[1]=_ip_tmp[1];
	ip_stub.ip_as_u8a[2]=_ip_tmp[2];
	ip_stub.ip_as_u8a[3]=_ip_tmp[3];
	return ip_stub.ip_as_u32_le;
}
void _mac_string_to_byte_array(const char * mac_string,uint8_t  *mac_stub)
{
	int _mac_tmp[6];
	sscanf(mac_string,"%x:%x:%x:%x:%x:%x",
		&_mac_tmp[0],
		&_mac_tmp[1],
		&_mac_tmp[2],
		&_mac_tmp[3],
		&_mac_tmp[4],
		&_mac_tmp[5]);
	
	mac_stub[0]=_mac_tmp[0];
	mac_stub[1]=_mac_tmp[1];
	mac_stub[2]=_mac_tmp[2];
	mac_stub[3]=_mac_tmp[3];
	mac_stub[4]=_mac_tmp[4];
	mac_stub[5]=_mac_tmp[5];
}
e3_type register_neighbor(e3_type service, e3_type ip_string,e3_type mac_string,e3_type pindex)
{
	const char * _ip_string=(char *)e3_type_to_uint8_t_ptr(ip_string);
    const char * _mac_string=(char *)e3_type_to_uint8_t_ptr(mac_string);
	uint16_t   * _pindex=(uint16_t*)e3_type_to_uint8_t_ptr(pindex);
	int index;
	
	uint8_t  mac_stub[6];
	uint32_t ip_stub=_ip_string_to_u32_le(_ip_string);
	_mac_string_to_byte_array(_mac_string,mac_stub);
	
	index=register_topological_neighbour(ip_stub,mac_stub);
	if(index<0)
		return -1;
	*_pindex=index;
	return 0;
}

DECLARE_E3_API(neighbor_registration)={
	.api_name="register_neighbor",
	.api_desc="register neighbor in neighborhood list",
	.api_callback_func=(api_callback_func)register_neighbor,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=16},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=18},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=2},
		{.type=e3_arg_type_none},
	},
};

e3_type refresh_neighbor_mac(e3_type service, e3_type ip_string,e3_type mac_string)
{
	const char * _ip_string=(char *)e3_type_to_uint8_t_ptr(ip_string);
    const char * _mac_string=(char *)e3_type_to_uint8_t_ptr(mac_string);
	uint8_t  mac_stub[6];
	uint32_t ip_stub=_ip_string_to_u32_le(_ip_string);
	_mac_string_to_byte_array(_mac_string,mac_stub);

	return update_neighbour_mac(ip_stub,mac_stub);
	return 0;
}

DECLARE_E3_API(neighbor_mac_update)={
	.api_name="refresh_neighbor_mac",
	.api_desc="update the mac address of a neighbor entry",
	.api_callback_func=(api_callback_func)refresh_neighbor_mac,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=16},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input,.len=18},
		{.type=e3_arg_type_none},
	},
	
};

#define MAX_NEIGHBORS_PER_FETCH 64

e3_type list_neighbours_partial(e3_type service,
							e3_type index_to_start,/*input&output*/
							e3_type nr_entries,/*output*/
							e3_type entries/*output*/)
{
	int    *  _index_to_start=(int*)e3_type_to_uint8_t_ptr(index_to_start);
	int    *  _nr_entries=(int*)e3_type_to_uint8_t_ptr(nr_entries);
	struct topological_neighbor * _entries=(struct topological_neighbor*)
											e3_type_to_uint8_t_ptr(entries);
	int idx=0;
	int iptr=0;
	for(idx=*_index_to_start;idx<MAX_TOPOLOGICAL_NEIGHBOURS;idx++){
		if(!topological_neighbor_base[idx].is_valid)
			continue;
		memcpy(&_entries[iptr++],
			&topological_neighbor_base[idx],
			sizeof(struct topological_neighbor));
		if(iptr==MAX_NEIGHBORS_PER_FETCH)
			break;
	}
	*_index_to_start=idx+1;
	*_nr_entries=iptr;
	return 0;
}

DECLARE_E3_API(partial_neighbour_list)={
	.api_name="list_neighbours_partial",
	.api_desc="enumerate partial list of neighbours",
	.api_callback_func=(api_callback_func)list_neighbours_partial,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_input_and_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=4},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,
		.len=sizeof(struct topological_neighbor)*MAX_NEIGHBORS_PER_FETCH},
		{.type=e3_arg_type_none},
	},
};