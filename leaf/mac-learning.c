#include <leaf/include/mac-learning.h>
#include <e3infra/include/node.h>
#include <e3infra/include/node-class.h>
#include <rte_malloc.h>
#include <e3infra/include/e3-log.h>
#include <e3infra/include/e3-init.h>
#include <e3infra/include/util.h>
#include <e3infra/include/lcore-extension.h>
void mac_learning_node_class_init(void)
{
    struct node_class * pclass=rte_zmalloc(NULL,sizeof(struct node_class),64);
    E3_ASSERT(pclass);
    sprintf((char*)pclass->class_name,MAC_LEARNING_CLASSNAME);
    pclass->class_reclaim_func=NULL;
    pclass->node_class_priv=NULL;
    E3_ASSERT(!register_node_class(pclass));
    E3_ASSERT(find_node_class_by_name(MAC_LEARNING_CLASSNAME)==pclass);
    E3_LOG("register node class:%s\n",(char*)pclass->class_name);
}
E3_init(mac_learning_node_class_init,TASK_PRIORITY_RESOURCE_INIT);

int mac_learning_node_process_func(void *arg)
{
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
    
    pnode->lcore_id=get_lcore_by_socket_id(socket_id);
    
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