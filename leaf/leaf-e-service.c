#include <leaf-e-service.h>
#include <rte_malloc.h>
#include <e3_init.h>
#include <e3_log.h>
struct ether_e_line * e_line_base;
struct ether_e_lan  * e_lan_base;
void init_e_service(void)
{
	e_line_base=rte_zmalloc(NULL,
			sizeof(struct ether_e_line)*MAX_E_LINE_SERVICES,
			64);
	E3_ASSERT(e_line_base);
	e_lan_base=rte_zmalloc(NULL,
			sizeof(struct ether_e_lan)*MAX_E_LAN_SERVICES,
			64);
	E3_ASSERT(e_lan_base);
}

E3_init(init_e_service,TASK_PRIORITY_RESOURCE_INIT);

/*
*register an e-line service,
*if successful, the actual index(which is greater than or equal to 0)
*is returned,otherwise, a negative value indicates failure
*/
int register_e_line_service(struct ether_e_line * eline)
{
	/*
	* check whether data fields conflict
	*/
	int idx=0;
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		if(!e_line_base[idx].is_valid)
			continue;
		if((e_line_base[idx].label_to_push==eline->label_to_push)&&
			(e_line_base[idx].NHLFE==eline->NHLFE))
			return -1;
		#if 0
			/*
			* a VLAN must be dedicated to a E-LINE service
			*/
			if((e_line_base[idx].e3iface==eline->e3iface)&&
				(e_line_base[idx].vlan_tci==eline->vlan_tci))
				return -2;
		#else
			if(e_line_base[idx].vlan_tci==eline->vlan_tci)
				return -2;
		#endif
	}
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		if(!e_line_base[idx].is_valid)
			break;
	if(idx>=MAX_E_LINE_SERVICES)
		return -3;
	if(reference_common_nexthop(eline->NHLFE))
		return -4;
	e_line_base[idx].index=idx;
	e_line_base[idx].e3iface=eline->e3iface;
	e_line_base[idx].label_to_push=eline->label_to_push;
	e_line_base[idx].NHLFE=eline->NHLFE;
	e_line_base[idx].vlan_tci=eline->vlan_tci;
	e_line_base[idx].ref_cnt=0;
	e_line_base[idx].is_valid=1;
	return idx;
}

/*
*find the target e-line service,
*and increment the ref count,0 returned upon success
*/
int reference_e_line_service(int index)
{
	struct ether_e_line * eline=find_e_line_service(index);
	if(!eline||!eline->is_valid)
		return -1;
	eline->ref_cnt++;
	return 0;
}

int dereference_e_line_service(int index)
{
	struct ether_e_line * eline=find_e_line_service(index);
	if(!eline||!eline->is_valid)
		return -1;
	if(eline->ref_cnt>0)
		eline->ref_cnt--;
	return 0;	
}

/*
*if the target entry is valid,and refernece count is 0,
*then mark is as invalid,and return 0,
*other cases will return non-zero
*/
int delete_e_line_service(int index)
{
	struct ether_e_line * eline=find_e_line_service(index);
	if(!eline||!eline->is_valid)
		return -1;
	if(eline->ref_cnt)/*still being used by other entity*/
		return -2;
	dereference_common_nexthop(eline->NHLFE);
	eline->is_valid=0;
	return 0;
}

int register_e_lan_service(void)
{
	int idx=0;
	for(idx=0;idx<MAX_E_LAN_SERVICES;idx++)
		if((!e_lan_base[idx].is_valid)||
			e_lan_base[idx].is_releasing)
			break;
	if(idx>=MAX_E_LAN_SERVICES)
		return -1;
	e_lan_base[idx].index=idx;
	e_lan_base[idx].multicast_label=0;
	e_lan_base[idx].multicast_NHLFE=-1;
	e_lan_base[idx].nr_ports=0;
	e_lan_base[idx].nr_nhlfes=0;
	e_lan_base[idx].ref_cnt=0;
	if(!(e_lan_base[idx].fib_base=allocate_findex_2_4_base()))
		return -2;
	e_lan_base[idx].is_releasing=0;
	e_lan_base[idx].is_valid=1;
	return idx;
}

int reference_e_lan_service(int index)
{
	struct ether_e_lan * elan=find_e_lan_service(index);
	if((!elan)||(!elan->is_valid))
		return -1;
	elan->ref_cnt++;
	return 0;
}
int dereference_e_lan_service(int index)
{
	struct ether_e_lan * elan=find_e_lan_service(index);
	if((!elan)||(!elan->is_valid))
		return -1;
	if(elan->ref_cnt>0)
		elan->ref_cnt--;
	return 0;
}
void post_delete_e_lan_service(struct rcu_head * rcu)
{
	int idx=0;
	struct ether_e_lan * elan=container_of(rcu,struct ether_e_lan,rcu);
	cleanup_findex_2_4_entries(elan->fib_base);
	rte_free(elan->fib_base);
	elan->fib_base=NULL;
	/*
	*process any relationship of resource
	*/
	dereference_common_nexthop(elan->multicast_NHLFE);
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
		if(!elan->nhlfes[idx].is_valid)
			continue;
		dereference_common_nexthop(elan->nhlfes[idx].NHLFE);
	}
	elan->is_releasing=0;/*mark it reuseable*/
}
int delete_e_lan_service(int index)
{
	struct ether_e_lan * elan=find_e_lan_service(index);
	if((!elan)||(!elan->is_valid))
		return -1;
	if(elan->ref_cnt)
		return -2;
	elan->is_releasing=1;
	elan->is_valid=0;
	call_rcu(&elan->rcu,post_delete_e_lan_service);
	return 0;
}
int register_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci)
{
	int idx=0;
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -1;
	/*
	*check arguments are leagal
	*/
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
		if(!elan->ports[idx].is_valid)
			continue;
		if((elan->ports[idx].vlan_tci==vlan_tci)&&
			(elan->ports[idx].iface==e3iface))
			return -2;
	}
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++)
		if(!elan->ports[idx].is_valid)
			break;
	if(idx>=MAX_PORTS_IN_E_LAN_SERVICE)
		return -2;
	elan->ports[idx].iface=e3iface;
	elan->ports[idx].vlan_tci=vlan_tci;
	elan->ports[idx].is_valid=1;
	elan->nr_ports++;
	E3_ASSERT(elan->nr_ports<=MAX_PORTS_IN_E_LAN_SERVICE);
	return idx=0;
}

int find_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci)
{
	int idx=0;
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -1;
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
		if(!elan->ports[idx].is_valid)
			continue;
		if((elan->ports[idx].iface==e3iface)&&
			(elan->ports[idx].vlan_tci==vlan_tci))
			return idx;
	}
	return -2;
}

int delete_e_lan_port(int elan_index,int port_index)
{
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -1;
	if((port_index<0)||
		(port_index>=MAX_PORTS_IN_E_LAN_SERVICE)||
		(!elan->ports[port_index].is_valid))
		return -2;
	/*
	*todo:delete mac entries 
	*which is relelated to this port
	*/

	elan->ports[port_index].is_valid=0;
	elan->nr_ports--;
	E3_ASSERT(elan->nr_ports>=0);
	return 0;
}
int register_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push)
{
	int idx=0;
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -1;
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
		if(!elan->nhlfes[idx].is_valid)
			continue;
		if((elan->nhlfes[idx].NHLFE==nhlfe)&&
			(elan->nhlfes[idx].label_to_push==label_to_push))
			return -2;
	}
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++)
		if(!elan->nhlfes[idx].is_valid)
			break;
	if(idx>=MAX_NHLFE_IN_E_LAN_SERVICE)
		return -3;
	if(reference_common_nexthop(nhlfe))
		return -4;
	elan->nhlfes[idx].NHLFE=nhlfe;
	elan->nhlfes[idx].label_to_push=label_to_push;
	elan->nhlfes[idx].is_valid=1;
	elan->nr_nhlfes++;
	E3_ASSERT(elan->nr_nhlfes<=MAX_NHLFE_IN_E_LAN_SERVICE);
	return idx;
}
int find_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push)
{
	int idx=0;
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -1;
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
		if(!elan->nhlfes[idx].is_valid)
			continue;
		if((elan->nhlfes[idx].NHLFE==nhlfe)&&
			(elan->nhlfes[idx].label_to_push==label_to_push))
			return idx;
	}
	return -2;
}
int delete_e_lan_nhlfe(int elan_index,int nhlfe_index)
{
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -1;
	
	if((nhlfe_index<0)||
		(nhlfe_index>=MAX_NHLFE_IN_E_LAN_SERVICE)||
		(!elan->nhlfes[nhlfe_index].is_valid))
		return -2;
	/*
	*todo:delete mac entries 
	*which is relelated to this nhlfe entry
	*/
	dereference_common_nexthop(elan->nhlfes[nhlfe_index].NHLFE);
	elan->nhlfes[nhlfe_index].is_valid=0;
	elan->nr_nhlfes--;
	E3_ASSERT(elan->nr_nhlfes>=0);
	return 0;
}
