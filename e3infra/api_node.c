#include <node.h>
#include <lcore_extension.h>
#include <e3_init.h>
#include <rte_malloc.h>
#include <e3_log.h>
#include <e3-api-wrapper.h>
#include <util.h>




int api_node_process_fun(void * arg)
{
	struct node * pnode=(struct node *)arg;
	struct e3_api_service *service=pnode->node_priv;
	
	if(e3_api_service_try_to_poll_request(service)>0){
		e3_api_service_dispatch_apis(service);
		e3_api_service_send_reponse(service);
	}
	return 0;
}


static int register_api_server_endpoint(char * bind_addr,int schedule_on_master)
{
	static int api_node_cnt=1;
	struct e3_api_service *service=NULL;
	struct node * pnode=rte_zmalloc(NULL,sizeof(struct node),64);
	E3_ASSERT(pnode);
	sprintf((char*)pnode->name,"api-node-%d",api_node_cnt++);
	pnode->lcore_id=schedule_on_master?
					rte_get_master_lcore():
					get_lcore();
	E3_ASSERT(validate_lcore_id(pnode->lcore_id));
	pnode->burst_size=NODE_BURST_SIZE;
	pnode->node_type=node_type_input;/*make it a fake input node, thus avoiding allocating ring buffer for the node*/
	pnode->node_process_func=api_node_process_fun;
	pnode->node_reclaim_func=NULL;//never reclaim it
	E3_ASSERT(!register_node(pnode));
	E3_ASSERT(find_node_by_name((char*)pnode->name)==pnode);
	E3_ASSERT(!attach_node_to_lcore(pnode));
	service=allocate_e3_api_service(bind_addr);
	E3_ASSERT(service);
	pnode->node_priv=service;
	E3_LOG("schedule api node:%s with endpoint %s on lcore %d\n",
				(char*)pnode->name,
				bind_addr,
				pnode->lcore_id);
	return 0;
}
void api_node_init(void)
{
	#if defined(ENABLE_MULTIPLE_API_ENDPOINT)
		register_api_server_endpoint("tcp://*:507",0);
		register_api_server_endpoint("tcp://*:508",0);
	#else
		register_api_server_endpoint("tcp://*:507",1);
	#endif
}

E3_init(api_node_init,TASK_PRIORITY_SYSTEM_INIT);


/*
*export API version interface
*input:
*	None
*ouput:
*	None
*return:
*	the version number
*/

e3_type e3datapath_version(e3_type service)
{
	return (e3_type)VERSION_DWORD(MAJOR_VERSION_NUMBER,
							MINOR_VERSION_NUMBER,
							BUILD_VERSION_NUMBER);
}

DECLARE_E3_API(e3_datapath_version)={
	.api_name="e3datapath_version",
	.api_desc="retrieve e3 datapath version dword",
	.api_callback_func=(api_callback_func)e3datapath_version,
	.args_desc={
		{.type=e3_arg_type_none,},
	},
};