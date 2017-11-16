/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/leaf-e-service.h>
#include <rte_malloc.h>
#include <e3infra/include/e3_init.h>
#include <e3infra/include/e3_log.h>
#include <e3infra/include/util.h>
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
			return -E3_ERR_ILLEGAL;
		#if 0
			/*
			* a VLAN must be dedicated to a E-LINE service
			*/
			if((e_line_base[idx].e3iface==eline->e3iface)&&
				(e_line_base[idx].vlan_tci==eline->vlan_tci))
				return -2;
		#else
			if(e_line_base[idx].vlan_tci==eline->vlan_tci)
				return -E3_ERR_ILLEGAL;
		#endif
	}
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		if(!e_line_base[idx].is_valid)
			break;
	if(idx>=MAX_E_LINE_SERVICES)
		return -E3_ERR_OUT_OF_RES;
	if(reference_common_nexthop(eline->NHLFE))
		return -E3_ERR_ILLEGAL;
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
		return -E3_ERR_ILLEGAL;
	eline->ref_cnt++;
	return 0;
}

int dereference_e_line_service(int index)
{
	struct ether_e_line * eline=find_e_line_service(index);
	if(!eline||!eline->is_valid)
		return -E3_ERR_ILLEGAL;
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
		return -E3_ERR_ILLEGAL;
	if(eline->ref_cnt)/*still being used by other entity*/
		return -E3_ERR_IN_USE;
	dereference_common_nexthop(eline->NHLFE);
	eline->is_valid=0;
	return E3_OK;
}

int register_e_lan_service(void)
{
	int idx=0;
	for(idx=0;idx<MAX_E_LAN_SERVICES;idx++)
		if((!e_lan_base[idx].is_valid)||
			e_lan_base[idx].is_releasing)
			break;
	if(idx>=MAX_E_LAN_SERVICES)
		return -E3_ERR_OUT_OF_RES;
	e_lan_base[idx].index=idx;
	e_lan_base[idx].multicast_label=0;
	e_lan_base[idx].multicast_NHLFE=-1;
	e_lan_base[idx].nr_ports=0;
	e_lan_base[idx].nr_nhlfes=0;
	e_lan_base[idx].ref_cnt=0;
	if(!(e_lan_base[idx].fib_base=allocate_findex_2_4_base()))
		return -E3_ERR_OUT_OF_MEM;
	e_lan_base[idx].is_releasing=0;
	e_lan_base[idx].is_valid=1;
	return idx;
}

int reference_e_lan_service(int index)
{
	struct ether_e_lan * elan=find_e_lan_service(index);
	if((!elan)||(!elan->is_valid))
		return -E3_ERR_ILLEGAL;
	elan->ref_cnt++;
	return E3_OK;
}
int dereference_e_lan_service(int index)
{
	struct ether_e_lan * elan=find_e_lan_service(index);
	if((!elan)||(!elan->is_valid))
		return -E3_ERR_ILLEGAL;
	if(elan->ref_cnt>0)
		elan->ref_cnt--;
	return E3_OK;
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
		return -E3_ERR_ILLEGAL;
	if(elan->ref_cnt)
		return -E3_ERR_IN_USE;
	elan->is_releasing=1;
	elan->is_valid=0;
	call_rcu(&elan->rcu,post_delete_e_lan_service);
	return E3_OK;
}
int register_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci)
{
	int idx=0;
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -E3_ERR_ILLEGAL;
	/*
	*check arguments are leagal
	*/
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
		if(!elan->ports[idx].is_valid)
			continue;
		if((elan->ports[idx].vlan_tci==vlan_tci)&&
			(elan->ports[idx].iface==e3iface))
			return -E3_ERR_ILLEGAL;
	}
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++)
		if(!elan->ports[idx].is_valid)
			break;
	if(idx>=MAX_PORTS_IN_E_LAN_SERVICE)
		return -E3_ERR_OUT_OF_RES;
	elan->ports[idx].iface=e3iface;
	elan->ports[idx].vlan_tci=vlan_tci;
	elan->ports[idx].is_valid=1;
	elan->nr_ports++;
	E3_ASSERT(elan->nr_ports<=MAX_PORTS_IN_E_LAN_SERVICE);
	return idx;
}

int find_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci)
{
	int idx=0;
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -E3_ERR_ILLEGAL;
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
		if(!elan->ports[idx].is_valid)
			continue;
		if((elan->ports[idx].iface==e3iface)&&
			(elan->ports[idx].vlan_tci==vlan_tci))
			return idx;
	}
	return -E3_ERR_GENERIC;
}

int delete_e_lan_port(int elan_index,int port_index)
{
    int is_skipping=0;
    int idx=0;
    int iptr=0;
    int tag_index;
    int tag_inner_index;
    int tag_length;
    struct findex_2_4_entry * pentry,
                            * pnext;
    struct e_lan_fwd_entry  fwd_entry;
    struct findex_2_4_key   key;
    e3_bitmap tmp_bitmap;
    
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -E3_ERR_ILLEGAL;
	if((port_index<0)||
		(port_index>=MAX_PORTS_IN_E_LAN_SERVICE)||
		(!elan->ports[port_index].is_valid))
		return -E3_ERR_ILLEGAL;
	/*
	*delete mac entries 
	*which is relelated to this port
	*/
    for(idx=0;idx<(1<<16);idx++){
        for(pentry=elan->fib_base[idx].next;pentry;pentry=pnext){
            pnext=pentry->next_entry;
            for(iptr=0,is_skipping=0;iptr<FINDEX_2_4_TAGS_LENGTH;iptr++){
                if(!e3_bitmap_is_bit_set(pentry->tag_avail,iptr))
                    continue;
                fwd_entry.entry_as_u64=pentry->values[iptr];
                if((!fwd_entry.is_port_entry)||
                   (fwd_entry.e3iface!=elan->ports[port_index].iface))
                    continue;
                tag_length=FINDEX_2_4_TAGS_LENGTH/MAX_2_4_TAGS_NR_OF_ENTRY;
                tag_index=iptr/tag_length;
                tag_inner_index=iptr-tag_index*tag_length;
                key.key_index=idx;
                key.tag1=pentry->tags[tag_index].tag1[tag_inner_index];
                key.tag2=pentry->tags[tag_index].tag2[tag_inner_index];

                tmp_bitmap=pentry->tag_avail;
                e3_bitmap_clear_bit(tmp_bitmap,iptr);
                is_skipping=!tmp_bitmap;
                delete_index_2_4_item_unsafe(elan->fib_base,&key);
                if(is_skipping)
                    break;
            }
        }
    }    
	elan->ports[port_index].is_valid=0;
	elan->nr_ports--;
	E3_ASSERT(elan->nr_ports>=0);
	return E3_OK;
}
int register_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push)
{
	int idx=0;
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -E3_ERR_ILLEGAL;
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
		if(!elan->nhlfes[idx].is_valid)
			continue;
		if((elan->nhlfes[idx].NHLFE==nhlfe)&&
			(elan->nhlfes[idx].label_to_push==label_to_push))
			return -E3_ERR_ILLEGAL;
	}
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++)
		if(!elan->nhlfes[idx].is_valid)
			break;
	if(idx>=MAX_NHLFE_IN_E_LAN_SERVICE)
		return -E3_ERR_OUT_OF_RES;
	if(reference_common_nexthop(nhlfe))
		return -E3_ERR_ILLEGAL;
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
		return -E3_ERR_ILLEGAL;
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
		if(!elan->nhlfes[idx].is_valid)
			continue;
		if((elan->nhlfes[idx].NHLFE==nhlfe)&&
			(elan->nhlfes[idx].label_to_push==label_to_push))
			return idx;
	}
	return -E3_ERR_GENERIC;
}
int delete_e_lan_nhlfe(int elan_index,int nhlfe_index)
{
    int is_skipping=0;
    int idx=0;
    int iptr=0;
    int tag_index;
    int tag_inner_index;
    int tag_length;
    struct findex_2_4_entry * pentry,
                            * pnext;
    struct e_lan_fwd_entry  fwd_entry;
    struct findex_2_4_key   key;
    e3_bitmap tmp_bitmap;
    
	struct ether_e_lan *elan=find_e_lan_service(elan_index);
	if((!elan)||(!elan->is_valid))
		return -E3_ERR_ILLEGAL;
	
	if((nhlfe_index<0)||
		(nhlfe_index>=MAX_NHLFE_IN_E_LAN_SERVICE)||
		(!elan->nhlfes[nhlfe_index].is_valid))
		return -E3_ERR_ILLEGAL;
	/*
	*delete mac entries 
	*which is relelated to this nhlfe entry
	*/
	 for(idx=0;idx<(1<<16);idx++){
        for(pentry=elan->fib_base[idx].next;pentry;pentry=pnext){
            pnext=pentry->next_entry;
            for(iptr=0,is_skipping=0;iptr<FINDEX_2_4_TAGS_LENGTH;iptr++){
                if(!e3_bitmap_is_bit_set(pentry->tag_avail,iptr))
                    continue;
                fwd_entry.entry_as_u64=pentry->values[iptr];
                if((fwd_entry.is_port_entry)||
                   (fwd_entry.NHLFE!=elan->nhlfes[nhlfe_index].NHLFE))
                    continue;
                tag_length=FINDEX_2_4_TAGS_LENGTH/MAX_2_4_TAGS_NR_OF_ENTRY;
                tag_index=iptr/tag_length;
                tag_inner_index=iptr-tag_index*tag_length;
                key.key_index=idx;
                key.tag1=pentry->tags[tag_index].tag1[tag_inner_index];
                key.tag2=pentry->tags[tag_index].tag2[tag_inner_index];

                tmp_bitmap=pentry->tag_avail;
                e3_bitmap_clear_bit(tmp_bitmap,iptr);
                is_skipping=!tmp_bitmap;
                delete_index_2_4_item_unsafe(elan->fib_base,&key);
                if(is_skipping)
                    break;
            }
        }
    }    
     
	dereference_common_nexthop(elan->nhlfes[nhlfe_index].NHLFE);
	elan->nhlfes[nhlfe_index].is_valid=0;
	elan->nr_nhlfes--;
	E3_ASSERT(elan->nr_nhlfes>=0);
	return 0;
}
int register_e_lan_fwd_entry(int elan_index,uint8_t * mac,struct e_lan_fwd_entry * fwd_entry)
{
    struct findex_2_4_key key;
    struct ether_e_lan * elan=find_e_lan_service(elan_index);
    if((!elan)||(!elan->is_valid))
        return -E3_ERR_ILLEGAL;
    if(fwd_entry->is_port_entry){
        if(find_e_lan_port(elan_index,fwd_entry->e3iface,fwd_entry->vlan_tci)<0)
            return -E3_ERR_NOT_FOUND;
    }else{
        if(find_e_lan_nhlfe(elan_index,fwd_entry->NHLFE,fwd_entry->label_to_push)<0)
            return -E3_ERR_NOT_FOUND;
    }
    mac_to_findex_2_4_key(mac,&key);
    key.value_as_u64=fwd_entry->entry_as_u64;
    if(add_index_2_4_item_unsafe(elan->fib_base,&key))/*out of memory*/
        return -E3_ERR_OUT_OF_MEM;
    return E3_OK;
}

int delete_e_lan_fwd_entry(int elan_index,uint8_t *mac)
{
    struct findex_2_4_key key;
    struct ether_e_lan * elan=find_e_lan_service(elan_index);
    if((!elan)||(!elan->is_valid))
        return -E3_ERR_ILLEGAL;
    mac_to_findex_2_4_key(mac,&key);
    delete_index_2_4_item_unsafe(elan->fib_base,&key);
    return E3_OK;
}

