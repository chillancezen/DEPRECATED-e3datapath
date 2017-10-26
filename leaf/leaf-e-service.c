#include <leaf-e-service.h>
#include <rte_malloc.h>
#include <e3_init.h>
#include <e3_log.h>
struct ether_e_line * e_line_base;

void init_e_service(void)
{
	e_line_base=rte_zmalloc(NULL,
			sizeof(struct ether_e_line)*MAX_E_LINE_SERVICES,
			64);
	E3_ASSERT(e_line_base);
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
		if((e_line_base[idx].e3iface==eline->e3iface)&&
			(e_line_base[idx].vlan_tci==eline->vlan_tci))
			return -2;
	}
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		if(!e_line_base[idx].is_valid)
			break;
	if(idx>=MAX_E_LINE_SERVICES)
		return -3;
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
	eline->is_valid=0;
	return 0;
}

