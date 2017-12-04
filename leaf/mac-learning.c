/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <leaf/include/mac-learning.h>
#include <e3infra/include/node.h>
#include <e3infra/include/node-class.h>
#include <rte_malloc.h>
#include <e3infra/include/e3-log.h>
#include <e3infra/include/e3-init.h>
#include <e3infra/include/util.h>
#include <e3infra/include/lcore-extension.h>
#include <rte_ring.h>
#include <rte_cycles.h>
struct node_class * pmac_learning_class;
void mac_learning_node_class_init(void)
{
    struct node_class * pclass=rte_zmalloc(NULL,sizeof(struct node_class),64);
    E3_ASSERT(pclass);
    sprintf((char*)pclass->class_name,MAC_LEARNING_CLASSNAME);
    pclass->class_reclaim_func=NULL;
    pclass->node_class_priv=NULL;
    E3_ASSERT(!register_node_class(pclass));
    E3_ASSERT(find_node_class_by_name(MAC_LEARNING_CLASSNAME)==pclass);
    E3_LOG("register node class:%s(class id:%d)\n",
		(char*)pclass->class_name,
		(int)pclass->node_class_index);
	pmac_learning_class=pclass;
}
E3_init(mac_learning_node_class_init,TASK_PRIORITY_RESOURCE_INIT);

int mac_learning_node_process_func(void *arg)
{
	struct node * pnode=(struct node*)arg;
	struct mac_learning_cache_entry cache_entry;
	struct ether_e_lan * elan;
	struct findex_2_4_key key;
	int idx=0;
	uint64_t ts_now=rte_get_tsc_cycles();
	for(idx=0;idx<16;idx++){
		if(rte_ring_sc_dequeue_bulk(pnode->node_ring,
			cache_entry.u64,
			2,
			NULL)!=2)
			break;
		/*
		*do updates,usually a new MAC entry should be 
		*inserted into E-LAN's fib base
		*/
		if(!(elan=find_e_lan_service(cache_entry.elan_index)))
			continue;
		mac_to_findex_2_4_key(cache_entry.mac,&key);
		key.leaf_fwd_entry_as_64=cache_entry.fwd_entry_as64;
		key.ts_last_updated=ts_now;
		get_e_lan(elan);
		add_index_2_4_item_unsafe(elan->fib_base,&key);
		put_e_lan(elan);
	}
    return 0;
}
int register_mac_learning_node(int socket_id,int node_id)
{
    struct node * pnode=rte_zmalloc_socket(NULL,sizeof(struct node),64,socket_id);
    if(!pnode){
        E3_ERROR("can not allocate mempory\n");
        return -E3_ERR_OUT_OF_MEM;
    }
    sprintf((char*)pnode->name,"mac-learn-%d.%d",socket_id,node_id);
    #if 0
    	pnode->lcore_id=get_lcore_by_socket_id(socket_id);
    #else
		pnode->lcore_id=rte_get_master_lcore();
	#endif
    if(!validate_lcore_id(pnode->lcore_id)){
        E3_ERROR("no valid lcore(this:%d) for node:%s\n",pnode->lcore_id,(char*)pnode->name);
        goto error_node_dealloc;
    }
    pnode->burst_size=NODE_BURST_SIZE;
    pnode->node_priv=NULL;
    pnode->node_type=node_type_worker;
    pnode->node_process_func=mac_learning_node_process_func;
    pnode->node_reclaim_func=default_rte_reclaim_func;

    if(register_node(pnode)){
        E3_ERROR("can not register node %s on lcore %d\n",(char*)pnode->name,pnode->lcore_id);
        put_lcore(pnode->lcore_id,0);
        goto error_node_dealloc;
    }

    E3_ASSERT(find_node_by_name((char*)pnode->name)==pnode);
    if(add_node_into_nodeclass(MAC_LEARNING_CLASSNAME,(char*)pnode->name)){
        E3_ERROR("can not add node %s into class %s\n",(char*)pnode->name,MAC_LEARNING_CLASSNAME);
        goto error_unregister_node;
    }
    if(attach_node_to_lcore(pnode)){
        E3_ERROR("can not attach %s to lcore\n",(char*)pnode->name);
        goto error_detach_node_from_class;
    }
    E3_LOG("register node %s on lcore %d\n",(char*)pnode->name,pnode->lcore_id);
    return E3_OK;
    error_detach_node_from_class:
        delete_node_from_nodeclass(MAC_LEARNING_CLASSNAME,(char*)pnode->name);
    error_unregister_node:
        put_lcore(pnode->lcore_id,0);
        unregister_node(pnode);
    error_node_dealloc:
        if(pnode)
            rte_free(pnode);
    return -E3_ERR_GENERIC;
}

void mac_learning_node_init(void)
{
    int socket_id;
    foreach_numa_socket(socket_id){
        E3_ASSERT(!register_mac_learning_node(socket_id,0));
    }
	
}
E3_init(mac_learning_node_init,TASK_PRIORITY_RESOURCE_INIT+1);