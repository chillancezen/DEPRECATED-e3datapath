/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/leaf-e-service.h>
#include <rte_malloc.h>
#include <e3infra/include/e3-init.h>
#include <e3infra/include/e3-log.h>
#include <e3infra/include/util.h>
#include <e3infra/include/malloc-wrapper.h>

struct ether_e_line * e_line_base;
struct ether_e_lan  * e_lan_base;

/*
*e_service_guard is shared by both e-line and e-lan services
*/
rte_rwlock_t e_line_guard;
rte_rwlock_t e_lan_guard;

#define RLOCK_ELINE() 	rte_rwlock_read_lock(&e_line_guard)
#define WLOCK_ELINE() 	rte_rwlock_write_lock(&e_line_guard)
#define RUNLOCK_ELINE() rte_rwlock_read_unlock(&e_line_guard)
#define WUNLOCK_ELINE() rte_rwlock_write_unlock(&e_line_guard)
#define RLOCK_ELAN() 	rte_rwlock_read_lock(&e_lan_guard)
#define WLOCK_ELAN() 	rte_rwlock_write_lock(&e_lan_guard)
#define RUNLOCK_ELAN() 	rte_rwlock_read_unlock(&e_lan_guard)
#define WUNLOCK_ELAN() 	rte_rwlock_write_unlock(&e_lan_guard)

/*
*only read locks are exported
*/
void __read_lock_eline()
{
	RLOCK_ELINE();
}
void __read_unlock_eline()
{
	RUNLOCK_ELINE();
}
void __read_lock_elan()
{
	RLOCK_ELAN();
}
void __read_unlock_elan()
{
	RUNLOCK_ELAN();
}

void init_e_service(void)
{
	
	e_line_base=RTE_ZMALLOC(NULL,
			sizeof(struct ether_e_line)*MAX_E_LINE_SERVICES,
			64);
	E3_ASSERT(e_line_base);
	e_lan_base=RTE_ZMALLOC(NULL,
			sizeof(struct ether_e_lan)*MAX_E_LAN_SERVICES,
			64);
	E3_ASSERT(e_lan_base);
	rte_rwlock_init(&e_line_guard);
	rte_rwlock_init(&e_lan_guard);
}

E3_init(init_e_service,TASK_PRIORITY_RESOURCE_INIT);

/*
*register an e-line service,
*if successful, the actual index(which is greater than or equal to 0)
*is returned,otherwise, a negative value indicates failure
*/
int register_e_line_service(void)
{
	int idx=0;
	int target=-E3_ERR_OUT_OF_RES;
	WLOCK_ELINE();
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++)
		if(!e_line_base[idx].is_valid)
			break;
	if(idx<MAX_E_LINE_SERVICES){
		target=idx;
		memset(&e_line_base[idx],0x0,sizeof(struct ether_e_line));
		e_line_base[idx].index=idx;
		e_line_base[idx].label_to_push=0;
		e_line_base[idx].NHLFE=-1;
		e_line_base[idx].e3iface=-1;
		e_line_base[idx].vlan_tci=0;
		e_line_base[idx].ref_cnt=0;
		e_line_base[idx].is_cbp_ready=0;
		e_line_base[idx].is_csp_ready=0;
		__sync_synchronize();
		e_line_base[idx].is_valid=1;
	}
	WUNLOCK_ELINE();
	E3_LOG("register e-line service with result :%d\n",target);
	return target;
}

int register_e_line_port(int eline_index,int e3iface,int vlan_tci)
{
	int ret=-E3_ERR_GENERIC;
	int idx=0;
	struct ether_e_line * eline;
	WLOCK_ELINE();
	if(!(eline=find_e_line_service(eline_index)))
		goto out;
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		if(idx==eline_index)
			continue;
		if(!e_line_base[idx].is_valid)
			continue;
		if((e_line_base[idx].e3iface==e3iface)&&
			(e_line_base[idx].vlan_tci==vlan_tci))
			goto out;/*no duplicated port in the E-line instance*/		
	}
	E3_ASSERT(!reference_e_line_service(eline_index));
	eline->e3iface=e3iface;
	eline->vlan_tci=vlan_tci;
	eline->is_csp_ready=1;
	ret=E3_OK;
	out:
	WUNLOCK_ELINE();
	E3_LOG("register e-line port entry:%d <e3iface:%d,vlan:%d> with result as %d\n",
		eline_index,
		e3iface,
		vlan_tci,
		ret);
	return ret;
}
int delete_e_line_port(int eline_index)
{
	int ret=-E3_ERR_GENERIC;
	struct ether_e_line * eline;
	WLOCK_ELINE();
	if((!(eline=find_e_line_service(eline_index)))||
		(!eline->is_csp_ready))
		goto out;
	E3_ASSERT(!dereference_e_line_service(eline_index));
	eline->is_csp_ready=0;
	__sync_synchronize();
	eline->e3iface=-1;
	eline->vlan_tci=0;
	ret=E3_OK;
	out:
	WUNLOCK_ELINE();
	E3_LOG("delete port entry of e-line service:%d with result as:%d\n",eline_index,ret);
	return ret;
}
int register_e_line_nhlfe(int eline_index,int NHLFE,int label_to_push)
{
	int ret=-E3_ERR_GENERIC;
	int idx=0;
	struct ether_e_line * eline;
	WLOCK_ELINE();
	if(!(eline=find_e_line_service(eline_index)))
		goto out;
	for(idx=0;idx<MAX_E_LINE_SERVICES;idx++){
		if(idx==eline_index)
			continue;
		if(!e_line_base[idx].is_valid)
			continue;
		if((e_line_base[idx].NHLFE==NHLFE)&&
			(e_line_base[idx].label_to_push==label_to_push))
			goto out;/*no duplicated nhlfe in the E-line instance*/	
	}
	if(reference_common_nexthop(NHLFE))
		goto out;
	/*release previous nhlfe binding*/
	if(eline->is_cbp_ready){
		dereference_common_nexthop(eline->NHLFE);
		eline->NHLFE=-1;
		eline->label_to_push=0;
		eline->is_cbp_ready=0;
	}
	eline->NHLFE=NHLFE;
	eline->label_to_push=label_to_push;
	eline->is_cbp_ready=1;
	ret=E3_OK;
	out:
	WUNLOCK_ELINE();
	E3_LOG("register e-line service nhlfe entry:%d <nhlfe:%d,label:%d> with result as %d\n",
		eline_index,
		NHLFE,
		label_to_push,
		ret);
	return ret;
}
int delete_e_line_nhlfe(int eline_index)
{
	int ret=-E3_ERR_GENERIC;
	struct ether_e_line * eline;
	WLOCK_ELINE();
	eline=find_e_line_service(eline_index);
	if((!eline)||
		(!eline->is_cbp_ready))
		goto out;
	eline->is_cbp_ready=0;
	__sync_synchronize();
	dereference_common_nexthop(eline->NHLFE);
	eline->NHLFE=-1;
	eline->label_to_push=0;
	ret=E3_OK;
	out:
	WUNLOCK_ELINE();
	E3_LOG("delete nhlfe entry of e-line service %d with result as :%d\n",eline_index,ret);
	return ret;
}

/*
*find the target e-line service,
*and increment the ref count,0 returned upon success
*/
int reference_e_line_service_locked(int index)
{
	int ret=-E3_ERR_GENERIC;
	WLOCK_ELINE();
	ret=reference_e_line_service(index);
	WUNLOCK_ELINE();
	return ret;
}
int reference_e_line_service(int index)
{
	int ret=-E3_ERR_GENERIC;
	struct ether_e_line * eline;
	if(!(eline=find_e_line_service(index)))
		goto out;
	eline->ref_cnt++;
	ret=E3_OK;
	out:
	return ret;
}

int dereference_e_line_service_locked(int index)
{
	int ret=-E3_ERR_GENERIC;
	WLOCK_ELINE();
	ret=dereference_e_line_service(index);
	WUNLOCK_ELINE();
	return ret;
}

int dereference_e_line_service(int index)
{
	int ret=-E3_ERR_GENERIC;
	struct ether_e_line * eline;
	if(!(eline=find_e_line_service(index)))
		goto out;
	if(eline->ref_cnt>0)
		eline->ref_cnt--;
	ret=E3_OK;
	out:
	return ret;	
}

/*
*if the target entry is valid,and refernece count is 0,
*then mark is as invalid,and return 0,
*other cases will return non-zero
*/
int delete_e_line_service(int index)
{	
	int ret=-E3_ERR_GENERIC;
	struct ether_e_line * eline;
	WLOCK_ELINE();
	if((!(eline=find_e_line_service(index)))||
		(eline->ref_cnt))/*it's still referenced by other entity*/
		goto out;
	if(eline->is_csp_ready){
		eline->vlan_tci=0;
		eline->e3iface=-1;
		eline->is_csp_ready=0;
	}
	if(eline->is_cbp_ready){
		dereference_common_nexthop(eline->NHLFE);
		eline->NHLFE=-1;
		eline->label_to_push=0;
		eline->is_cbp_ready=0;
	}
	__sync_synchronize();
	eline->is_valid=0;
	ret=E3_OK;
	out:
	WUNLOCK_ELINE();
	E3_LOG("delete e-line service:%d with result:%d\n",index,ret);
	return ret;
}







int register_e_lan_service(void)
{
	int ret=-E3_ERR_OUT_OF_RES;
	int idx=0;
	WLOCK_ELAN();
	for(idx=0;idx<MAX_E_LAN_SERVICES;idx++)
		if((!e_lan_base[idx].is_valid)&&
			(!e_lan_base[idx].is_releasing))
			break;
	if(idx>=MAX_E_LAN_SERVICES)
		goto out;
	memset(&e_lan_base[idx],0x0,sizeof(struct ether_e_lan));
	e_lan_base[idx].index=idx;
	e_lan_base[idx].multicast_label=0;
	e_lan_base[idx].multicast_NHLFE=-1;
	e_lan_base[idx].nr_ports=0;
	e_lan_base[idx].nr_nhlfes=0;
	e_lan_base[idx].ref_cnt=0;
	if(!(e_lan_base[idx].fib_base=allocate_findex_2_4_base())){
		ret=-E3_ERR_OUT_OF_MEM;
		goto out;
	}
	e_lan_base[idx].is_releasing=0;
	e_lan_base[idx].is_valid=1;
	rte_spinlock_init(&e_lan_base[idx].per_e_lan_guard);
	ret=idx;
	out:
	WUNLOCK_ELAN();
	E3_LOG("register E-LAN service with result as %d\n",ret);
	return ret;
}

int reference_e_lan_service_locked(int index)
{
	int ret=-E3_ERR_ILLEGAL;
	WLOCK_ELAN();
	ret=reference_e_lan_service(index);
	WUNLOCK_ELAN();
	return ret;
}
int reference_e_lan_service(int index)
{	
	int ret=-E3_ERR_ILLEGAL;
	struct ether_e_lan *elan; 
	elan=find_e_lan_service(index);
	if(!elan)
		goto out;
	elan->ref_cnt++;
	ret=E3_OK;
	out:
	return ret;
}

int dereference_e_lan_service_locked(int index)
{
	int ret=-E3_ERR_ILLEGAL;
	WLOCK_ELAN();
	ret=dereference_e_lan_service(index);
	WUNLOCK_ELAN();
	return ret;
}

int dereference_e_lan_service(int index)
{
	int ret=-E3_ERR_ILLEGAL;	
	struct ether_e_lan * elan;
	elan=find_e_lan_service(index);
	if(!elan)
		goto out;
	if(elan->ref_cnt>0)
		elan->ref_cnt--;
	ret=E3_OK;
	out:
	return ret;
}
static void post_delete_e_lan_service(struct rcu_head * rcu)
{
	int idx=0;
	struct ether_e_lan * elan=container_of(rcu,struct ether_e_lan,rcu);
	cleanup_findex_2_4_entries(elan->fib_base);
	RTE_FREE(elan->fib_base);
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
	/*mark it reuseable*/
	elan->is_releasing=0;
	E3_LOG("E-LAN service:%d is released\n",elan->index);
}
int delete_e_lan_service(int index)
{
	int ret=-E3_ERR_ILLEGAL;
	struct ether_e_lan * elan;
	WLOCK_ELAN();
	elan=find_e_lan_service(index);
	if(!elan)
		goto out;
	if(elan->ref_cnt){
		ret=-E3_ERR_IN_USE;
		goto out;
	}
	elan->is_releasing=1;
	elan->is_valid=0;
	call_rcu(&elan->rcu,post_delete_e_lan_service);
	ret=E3_OK;
	out:
	WUNLOCK_ELAN();
	E3_LOG("delete E-LAN service %d with result as :%d\n",index,ret);
	return ret;
}

int register_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci)
{
	int ret=-E3_ERR_ILLEGAL;
	int idx=0;
	struct ether_e_lan *elan;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_index);
	if(!elan)
		goto out;
	/*
	*check whether arguments are leagal
	*/
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
		if(!elan->ports[idx].is_valid)
			continue;
		if((elan->ports[idx].vlan_tci==vlan_tci)&&
			(elan->ports[idx].iface==e3iface))
			goto out;
	}
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++)
		if(!elan->ports[idx].is_valid)
			break;
	if(idx>=MAX_PORTS_IN_E_LAN_SERVICE){
		ret=-E3_ERR_OUT_OF_RES;
		goto out;
	}
	E3_ASSERT(!reference_e_lan_service(elan_index));
	elan->ports[idx].iface=e3iface;
	elan->ports[idx].vlan_tci=vlan_tci;
	elan->ports[idx].is_valid=1;
	elan->nr_ports++;
	E3_ASSERT(elan->nr_ports<=MAX_PORTS_IN_E_LAN_SERVICE);
	ret=idx;
	out:
	WUNLOCK_ELAN();
	E3_LOG("register E-LAN service %d port entry <iface:%d,vlan:%d> as result as %d\n",
		elan_index,
		e3iface,
		vlan_tci,
		ret);
	return ret;
}

int find_e_lan_port_locked(int elan_index,uint16_t e3iface,uint16_t vlan_tci)
{
	int rc;
	RLOCK_ELAN();
	rc=find_e_lan_port(elan_index,e3iface,vlan_tci);
	RUNLOCK_ELAN();
	return rc;
}

int find_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci)
{
	int ret=-E3_ERR_ILLEGAL;
	int idx=0;
	struct ether_e_lan *elan;
	elan=find_e_lan_service(elan_index);
	if(!elan)
		goto out;
	for(idx=0;idx<MAX_PORTS_IN_E_LAN_SERVICE;idx++){
		if(!elan->ports[idx].is_valid)
			continue;
		if((elan->ports[idx].iface==e3iface)&&
			(elan->ports[idx].vlan_tci==vlan_tci)){
			ret=idx;
			break;
		}
	}
	out:
	return ret;
}

int delete_e_lan_port(int elan_index,int port_index)
{
	int ret=-E3_ERR_GENERIC;
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
    
	struct ether_e_lan *elan;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_index);
	if(!elan)
		goto out;
	if((port_index<0)||
		(port_index>=MAX_PORTS_IN_E_LAN_SERVICE)||
		(!elan->ports[port_index].is_valid))
		goto out;
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
                fwd_entry.entry_as_u64=pentry->_values[iptr].leaf_fwd_entry_as_64;
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
	E3_ASSERT(!dereference_e_lan_service(elan_index));
	elan->ports[port_index].is_valid=0;
	elan->nr_ports--;
	E3_ASSERT(elan->nr_ports>=0);

	ret=E3_OK;
	out:
	WUNLOCK_ELAN();
	E3_LOG("delete E-LAN service %d port entry %d with result as:%d\n",
		elan_index,
		port_index,
		ret);
	return ret;
}
int register_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push)
{
	int ret=-E3_ERR_ILLEGAL;
	int idx=0;
	struct ether_e_lan *elan;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_index);
	if(!elan)
		goto out;
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
		if(!elan->nhlfes[idx].is_valid)
			continue;
		if((elan->nhlfes[idx].NHLFE==nhlfe)&&
			(elan->nhlfes[idx].label_to_push==label_to_push))
			goto out;
	}
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++)
		if(!elan->nhlfes[idx].is_valid)
			break;
	if(idx>=MAX_NHLFE_IN_E_LAN_SERVICE)
		goto out;
	if(reference_common_nexthop(nhlfe))
		goto out;
	elan->nhlfes[idx].NHLFE=nhlfe;
	elan->nhlfes[idx].label_to_push=label_to_push;
	elan->nhlfes[idx].is_valid=1;
	elan->nr_nhlfes++;
	E3_ASSERT(elan->nr_nhlfes<=MAX_NHLFE_IN_E_LAN_SERVICE);
	ret=idx;
	out:
	WUNLOCK_ELAN();
	E3_LOG("register E-LAN service %d nhlfe entry<nexthop:%d,label:%d> with result as %d\n",
		elan_index,
		nhlfe,
		label_to_push,
		ret);
	return ret;
}
int find_e_lan_nhlfe_locked(int elan_index,uint16_t nhlfe,uint32_t label_to_push)
{
	int rc;
	RLOCK_ELAN();
	rc=find_e_lan_nhlfe(elan_index,nhlfe,label_to_push);
	RUNLOCK_ELAN();
	return rc;
}
int find_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push)
{
	int ret=-E3_ERR_ILLEGAL;
	int idx=0;
	struct ether_e_lan *elan;
	elan=find_e_lan_service(elan_index);
	if(!elan)
		goto out;
	for(idx=0;idx<MAX_NHLFE_IN_E_LAN_SERVICE;idx++){
		if(!elan->nhlfes[idx].is_valid)
			continue;
		if((elan->nhlfes[idx].NHLFE==nhlfe)&&
			(elan->nhlfes[idx].label_to_push==label_to_push)){
			ret=idx;
			break;
		}
	}
	out:
	return ret;
}
int delete_e_lan_nhlfe(int elan_index,int nhlfe_index)
{
	int ret=-E3_ERR_ILLEGAL;
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
    
	struct ether_e_lan *elan;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_index);
	if(!elan)
		goto out;
	if((nhlfe_index<0)||
		(nhlfe_index>=MAX_NHLFE_IN_E_LAN_SERVICE)||
		(!elan->nhlfes[nhlfe_index].is_valid))
		goto out;
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
                fwd_entry.entry_as_u64=pentry->_values[iptr].leaf_fwd_entry_as_64;
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
	ret=E3_OK;
	out:
	WUNLOCK_ELAN();
	E3_LOG("delete E-LAN service:%d nhlfe entry %d with result as %d\n",
		elan_index,
		nhlfe_index,
		ret);
	return ret;
}
int register_e_lan_fwd_entry(int elan_index,uint8_t * mac,struct e_lan_fwd_entry * fwd_entry)
{
	int ret=-E3_ERR_ILLEGAL;
    struct findex_2_4_key key;
    struct ether_e_lan * elan;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_index);
    if(!elan)
        goto out;
    if(fwd_entry->is_port_entry){
        if(find_e_lan_port(elan_index,fwd_entry->e3iface,fwd_entry->vlan_tci)<0){
			ret=-E3_ERR_NOT_FOUND;
			goto out;
        }
    }else{
        if(find_e_lan_nhlfe(elan_index,fwd_entry->NHLFE,fwd_entry->label_to_push)<0){
			ret=-E3_ERR_NOT_FOUND;
			goto out;
        }
    }
    mac_to_findex_2_4_key(mac,&key);
    key.leaf_fwd_entry_as_64=fwd_entry->entry_as_u64;
    if(add_index_2_4_item_unsafe(elan->fib_base,&key))/*out of memory*/{
		ret=-E3_ERR_OUT_OF_MEM;
		goto out;
    }
	ret=E3_OK;
	out:
	WUNLOCK_ELAN();
    return ret;
}

int delete_e_lan_fwd_entry(int elan_index,uint8_t *mac)
{	
	int ret=-E3_ERR_ILLEGAL;
    struct findex_2_4_key key;
    struct ether_e_lan * elan;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_index);
    if(!elan)
    	goto out;
    mac_to_findex_2_4_key(mac,&key);
    delete_index_2_4_item_unsafe(elan->fib_base,&key);
	ret=E3_OK;
	out:
	WUNLOCK_ELAN();
    return ret;
}
int set_e_lan_multicast_fwd_entry(int16_t elan_idx,int16_t nhlfe_index,int32_t label_to_push)
{
	int ret=-E3_ERR_GENERIC;
	struct ether_e_lan * elan=NULL;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_idx);
	if(!elan){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	if(find_common_nexthop(elan->multicast_NHLFE))
		dereference_common_nexthop(elan->multicast_NHLFE);
	/*
	*invalidate multicast_NHLFE temporarily
	*/
	elan->multicast_NHLFE=-1;
	if(reference_common_nexthop(nhlfe_index)){
		ret=-E3_ERR_ILLEGAL;
		goto out;
	}
	elan->multicast_label=label_to_push;
	elan->multicast_NHLFE=nhlfe_index;
	ret=E3_OK;
	out:
	WUNLOCK_ELAN();
	E3_LOG("set E-LAN service %d multicast NHLFE <nexthop:%d,label:%d> with result %d\n",
		elan_idx,
		nhlfe_index,
		label_to_push,
		ret);
	return ret;
}
int reset_e_lan_multicast_fwd_entry(int16_t elan_idx)
{
	int ret=-E3_ERR_GENERIC;
	struct ether_e_lan * elan=NULL;
	WLOCK_ELAN();
	elan=find_e_lan_service(elan_idx);
	if(!elan){
		ret=-E3_ERR_NOT_FOUND;
		goto out;
	}
	if(find_common_nexthop(elan->multicast_NHLFE))
		dereference_common_nexthop(elan->multicast_NHLFE);
	elan->multicast_NHLFE=-1;
	elan->multicast_label=0;
	ret=E3_OK;
	out:
	WUNLOCK_ELAN();
	E3_LOG("reset E-LAN service %d multicast NHLFE with result as %d\n",
		elan_idx,
		ret);
	return ret;
}
