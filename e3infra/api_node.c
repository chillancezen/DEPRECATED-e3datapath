#include <node.h>
#include <lcore_extension.h>
#include <e3_init.h>
#include <e3_config.h>
#include <e3_log.h>






static int register_api_server_endpoint(const char * bind_addr)
{
	struct node * pnode=rte_zmalloc(NULL,sizeof(struct node),64);
	E3_ASSERT(pnode);
	
	return 0;
}
void api_node_init(void)
{
	
}

E3_init(api_node_init,TASK_PRIORITY_SYSTEM_INIT);




