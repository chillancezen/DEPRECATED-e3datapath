/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <rte_malloc.h>
#include <e3infra/include/e3interface.h>
#include <string.h>
#include <e3infra/include/node.h>
#include <e3infra/include/lcore-extension.h>
#include <e3infra/include/node-adjacency.h>
#include <e3api/include/e3-api-wrapper.h>
#include <e3net/include/e3iface-inventory.h>
#include <e3infra/include/malloc-wrapper.h>
//#define USE_NUMA_NODE 
/*when allocating lcore resource to io nodes,
if we define this macro,as a matter of optimization,user should know which socket 
their pci device belongs to,
that's to say,user must priviously decide the numa node,often through the EAL output*/

struct E3Interface * global_e3iface_array[MAX_NUMBER_OF_E3INTERFACE];

static void input_and_output_reclaim_func(struct rcu_head * rcu)
{
	struct node *pnode=container_of(rcu,struct node,rcu);
	E3_LOG("delete E3Interface's node %p with name %s\n",pnode,(char*)pnode->name);
	RTE_FREE(pnode);
}

struct E3Interface * alloc_e3interface(int priv_size,int socket_id)
{
	int alloc_size=sizeof(struct E3Interface)+priv_size;
	if(alloc_size&0x3f)
		alloc_size=(alloc_size&(~0x3f))+0x40;
	struct E3Interface * e3iface=RTE_ZMALLOC(NULL,alloc_size,socket_id);
	E3_LOG("allocate E3Interface :%p\n",e3iface);
	return e3iface;
}
void dealloc_e3interface(struct E3Interface * pif)
{
	if(pif){
		E3_LOG("deallocate E3Interface:%p with name %s\n",pif,(char*)pif->name)
		RTE_FREE(pif);
	}
}
char * link_speed_to_string(uint32_t speed)
{
	char *ret="Ethernet";
	switch(speed)
	{
	#define _(s) \
		case ETH_SPEED_NUM_##s: \
			ret=#s"Ethernet"; \
			break;
		_(10M);
		_(100M);
		_(1G);
		_(2_5G);
		_(5G);
		_(10G);
		_(20G);
		_(25G);
		_(40G);
		_(50G);
		_(56G);
		_(100G);
	#undef _	
	}
	
	return ret;
}

static int e3_lsc_callback(uint8_t iface,
			enum rte_eth_event_type type,
			void * param,
			void * ret_param)
{
	struct rte_eth_link link;
	struct E3Interface * pe3iface;
	if(type!=RTE_ETH_EVENT_INTR_LSC)
		return 0;
	pe3iface=find_e3interface_by_index(iface);
	E3_ASSERT(pe3iface&&(pe3iface->port_id==iface));
	rte_eth_link_get_nowait(iface,&link);
	if(link.link_status){
		E3_LOG("lsc notification:interface %s(port %d) up\n",
			(char*)pe3iface->name,
			pe3iface->port_id);
		if(pe3iface->interface_up)
			pe3iface->interface_up(iface);
	}else{
		E3_LOG("lsc notification:interface %s(port %d) down\n",
			(char*)pe3iface->name,
			pe3iface->port_id);
		if(pe3iface->interface_down)
			pe3iface->interface_down(iface);
	}
	return 0;
}
int register_e3interface(const char * params,struct E3Interface_ops * dev_ops,int *pport_id)
{
	int rc;
	int idx=0;
	int iptr;
	struct E3Interface *pe3iface=NULL;
	struct rte_eth_conf port_conf;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_link dev_link;
	struct node * pinput_nodes[MAX_QUEUES_TO_POLL];
	struct node * poutput_nodes[MAX_QUEUES_TO_POLL];
	struct rte_mempool * mempool=NULL;
	
	uint8_t port_id=0;
	memset(pinput_nodes,0x0,sizeof(pinput_nodes));
	memset(poutput_nodes,0x0,sizeof(poutput_nodes));
	
	rc=rte_eth_dev_attach(params,&port_id);
	if(rc){
		E3_ERROR("creating E3Interface with param:%s fails\n",params);
		return -1;
	}
	pe3iface=alloc_e3interface(dev_ops->priv_size,dev_ops->numa_socket_id);
	if(!pe3iface){
		E3_ERROR("allocting E3interface fails\n");
		goto error_dev_detach;
	}

	pe3iface->port_id=port_id;
	/*1.do capability check*/
	if(dev_ops->capability_check && dev_ops->capability_check(port_id)){
		E3_ERROR("checking capability for:%s fails\n",params);
		goto error_iface_dealloc;
	}
	/*2.do setup before go further*/
	dev_ops->queue_setup(pe3iface);
	if(pe3iface->nr_queues<=0 || pe3iface->nr_queues>MAX_QUEUES_TO_POLL){
		E3_ERROR("invalid number of queues to poll during queue setup\n");
		goto error_iface_dealloc;
	}
	/*3.allocate nodes*/
	for(idx=0;idx<MAX_QUEUES_TO_POLL;idx++){
		pe3iface->input_node[idx]=0;
		pe3iface->output_node[idx]=0;
		pinput_nodes[idx]=NULL;
	}
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		pinput_nodes[idx]=RTE_ZMALLOC(NULL,sizeof(struct node),64);
		poutput_nodes[idx]=RTE_ZMALLOC(NULL,sizeof(struct node),64);
		if(!pinput_nodes[idx] || !poutput_nodes[idx])
			break;
	}
	if(idx<pe3iface->nr_queues){
		E3_ERROR("no enough memory for nodes allocation\n");
		goto error_nodes_dealloc;
	}
	/*4. nodes setup*/
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		sprintf((char*)pinput_nodes[idx]->name,"dev-input-node-%d.%d",port_id,idx);
		sprintf((char*)poutput_nodes[idx]->name,"dev-output-node-%d.%d",port_id,idx);
		pinput_nodes[idx]->node_process_func=dev_ops->input_node_process_func;
		poutput_nodes[idx]->node_process_func=dev_ops->output_node_process_func;
		pinput_nodes[idx]->node_type=node_type_input;
		poutput_nodes[idx]->node_type=node_type_output;
		pinput_nodes[idx]->burst_size=NODE_BURST_SIZE;
		poutput_nodes[idx]->burst_size=NODE_BURST_SIZE;
		pinput_nodes[idx]->node_priv=(void*)MAKE_UINT64(port_id,idx);
		poutput_nodes[idx]->node_priv=(void*)MAKE_UINT64(port_id,idx);
		pinput_nodes[idx]->node_reclaim_func=NULL;
		poutput_nodes[idx]->node_reclaim_func=NULL;
	}
	/*5.determine the cores to be attached for every nodes,
	the principle is the input&output for a queue is attached to the nodes
	which belong to the same numa node*/
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		int input_lcore_id;
		int output_lcore_id;
		int node_socket_id;
		pinput_nodes[idx]->lcore_id=0xff;
		poutput_nodes[idx]->lcore_id=0xff;
		#if !defined(USE_NUMA_NODE)
			input_lcore_id=get_io_lcore();
		#else
			input_lcore_id=get_io_lcore_by_socket_id(dev_ops->numa_socket_id);
		#endif
		if(!validate_lcore_id(input_lcore_id)){
			for(iptr=0;iptr<idx;iptr++){
				put_lcore(pinput_nodes[iptr]->lcore_id,1);
				put_lcore(poutput_nodes[iptr]->lcore_id,1);
			}
			break;
		}
		node_socket_id=lcore_to_socket_id(input_lcore_id);
		output_lcore_id=get_io_lcore_by_socket_id(node_socket_id);
		if(!validate_lcore_id(output_lcore_id)){
			put_lcore(output_lcore_id,1);
			/*release previous allocated lcore resource*/
			for(iptr=0;iptr<idx;iptr++){
				put_lcore(pinput_nodes[iptr]->lcore_id,1);
				put_lcore(poutput_nodes[iptr]->lcore_id,1);
			}
			break;
		}
		pinput_nodes[idx]->lcore_id=input_lcore_id;
		poutput_nodes[idx]->lcore_id=output_lcore_id;
	}
	if(idx<pe3iface->nr_queues){
		E3_ERROR("errors occur during lcore allocation\n");
		goto error_nodes_dealloc;
	}
	/*6.register these nodes*/
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		if(register_node(pinput_nodes[idx]))
			break;
		if(register_node(poutput_nodes[idx]))
			break;
	}
	if(idx<pe3iface->nr_queues){
		E3_ERROR("registering nodes fails\n");
		goto error_nodes_unregister;
	}
	/*7.configure port by call port_setup() in dev_ops*/
	memset(&port_conf,0x0,sizeof(struct rte_eth_conf));
	dev_ops->port_setup(pe3iface,&port_conf);
	port_conf.intr_conf.lsc=!!dev_ops->check_lsc;
	rc=rte_eth_dev_configure(port_id,pe3iface->nr_queues,pe3iface->nr_queues,&port_conf);
	if(rc<0){
		E3_ERROR("errors occur during configuring dpdk port\n");
		goto error_nodes_unregister;
	}
	if(dev_ops->check_lsc){
		/*as a matter of fact, if the interface is configured to check lsc,and the interface does support
		it, this process is not going to fail*/
		rc=rte_eth_dev_callback_register(port_id,RTE_ETH_EVENT_INTR_LSC,e3_lsc_callback,NULL);
		if(rc<0){
			E3_ERROR("errors occur during registering LSC callback\n");
			goto error_nodes_unregister;
		}
		pe3iface->lsc_enabled=1;
		pe3iface->interface_up=dev_ops->lsc_iface_up;
		pe3iface->interface_down=dev_ops->lsc_iface_down;
	}
	/*rte_eth_promiscuous_enable(port_id); 
	not all the iterfaces work in promiscuous state,
	if we really need this, please do it in post_setup
	*/
	rte_eth_dev_info_get(port_id,&dev_info);
	dev_info.default_txconf.txq_flags=0;
	
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		mempool=get_mempool_by_socket_id(lcore_to_socket_id(pinput_nodes[idx]->lcore_id));
		if(!mempool)
			break;
		rc=rte_eth_rx_queue_setup(port_id,
			idx,
			DEFAULT_RX_DESCRIPTOR,
			lcore_to_socket_id(pinput_nodes[idx]->lcore_id),
			&dev_info.default_rxconf,
			mempool);
		if(rc<0)
			break;
		rc=rte_eth_tx_queue_setup(port_id,
			idx,
			DEFAULT_TX_DESCRIPTOR,
			lcore_to_socket_id(poutput_nodes[idx]->lcore_id),
			&dev_info.default_txconf);
		if(rc<0)
			break;
	}
	if(idx<pe3iface->nr_queues){
		E3_ERROR("errors occur during rx/tx queues setup\n");
		goto error_nodes_unregister;
	}
	/*8.fill the interface's name and record mac*/
	rte_eth_link_get_nowait(port_id,&dev_link);
	{
		char dev_data_name[64];
		char * lptr;
		rte_eth_dev_get_name_by_port(port_id,dev_data_name);
		lptr=dev_data_name;
		if(*lptr<='z'&&*lptr>='a')
			*lptr+='A'-'a';
		while(*lptr){
			if(*lptr==':'||*lptr=='.')
				*lptr='/';
			lptr++;
		}
		sprintf((char*)pe3iface->name,"%s%s",link_speed_to_string(dev_link.link_speed),dev_data_name);
	}
	{
		struct ether_addr mac_addr;
		rte_eth_macaddr_get(port_id,&mac_addr);
		memcpy(pe3iface->mac_addrs,mac_addr.addr_bytes,6);
	}
	/*9.prepare the nodes to RUN*/
	set_status_down_e3iface(pe3iface);
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		if(attach_node_to_lcore(pinput_nodes[idx]))
			break;
		if(attach_node_to_lcore(poutput_nodes[idx]))
			break;
	}
	if(idx<pe3iface->nr_queues){
		E3_ERROR("errors occur during lcore attaching phaze\n");
		goto error_lcore_detach;
	}
	/*10.set next edges for input nodes*/
	for(idx=0;(idx<MAX_PREDEFINED_EDGE)&&
				(dev_ops->edges[idx].edge_entry>=0)&&
				  (dev_ops->edges[idx].edge_entry<MAX_NR_EDGES);idx++){
		switch(dev_ops->edges[idx].fwd_behavior)
		{
			case NODE_TO_NODE_FWD:
				for(iptr=0;iptr<pe3iface->nr_queues;iptr++)
					if(set_node_to_node_edge((char*)pinput_nodes[iptr]->name,
						dev_ops->edges[idx].edge_entry,
						dev_ops->edges[idx].next_ref))
						goto error_edge_setup;
				break;
			case NODE_TO_CLASS_FWD:
				for(iptr=0;iptr<pe3iface->nr_queues;iptr++)
					if(set_node_to_class_edge((char*)pinput_nodes[iptr]->name,
						dev_ops->edges[idx].edge_entry,
						dev_ops->edges[idx].next_ref))
						goto error_edge_setup;
				break;
			default:
				break;
		}
		continue;
		error_edge_setup:
			E3_ERROR("errors occurs during next edges setup\n");
			goto error_lcore_detach;
	}
	/*11.do post-setup calling*/
	if(dev_ops->post_setup){
		if(dev_ops->post_setup(pe3iface)){
			E3_ERROR("errors occurs during post setting-up\n");
			goto error_lcore_detach;
		}
	}
	/*12. misc non-error setup*/
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		pinput_nodes[idx]->node_reclaim_func=input_and_output_reclaim_func;
		poutput_nodes[idx]->node_reclaim_func=input_and_output_reclaim_func;
		pe3iface->input_node[idx]=pinput_nodes[idx]->node_index;
		pe3iface->output_node[idx]=poutput_nodes[idx]->node_index;
	}
	E3_LOG("register E3interface:%s with ifindex:%d\n",(char*)pe3iface->name,
			pe3iface->port_id);
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		E3_LOG("\tnode %s on %d and node %s on %d\n",
			(char*)pinput_nodes[idx]->name,
			(int)pinput_nodes[idx]->lcore_id,
			(char*)poutput_nodes[idx]->name,
			(int)poutput_nodes[idx]->lcore_id);
	}
	if(pport_id)
		*pport_id=(int)port_id;
	pe3iface->under_releasing=0;
	pe3iface->interface_delete=dev_ops->iface_delete;
	/*13.publish this E3interface*/
	rcu_assign_pointer(global_e3iface_array[port_id],pe3iface);

	return 0;
	error_lcore_detach:
		for(idx=0;idx<pe3iface->nr_queues;idx++){
			detach_node_from_lcore(pinput_nodes[idx]);
			detach_node_from_lcore(poutput_nodes[idx]);
		}
	error_nodes_unregister:
		for(idx=0;idx<pe3iface->nr_queues;idx++){
			unregister_node(pinput_nodes[idx]);
			unregister_node(poutput_nodes[idx]);
		}
		for(idx=0;idx<pe3iface->nr_queues;idx++){
			if(validate_lcore_id(pinput_nodes[idx]->lcore_id))
				put_lcore(pinput_nodes[idx]->lcore_id,1);
			if(validate_lcore_id(poutput_nodes[idx]->lcore_id))
				put_lcore(poutput_nodes[idx]->lcore_id,1);
		}
	error_nodes_dealloc:
		for(idx=0;idx<pe3iface->nr_queues;idx++){
			if(pinput_nodes[idx])
				RTE_FREE(pinput_nodes[idx]);
			if(poutput_nodes[idx])
				RTE_FREE(poutput_nodes[idx]);
		}
	error_iface_dealloc:
		if(pe3iface&&pe3iface->lsc_enabled){
			rte_eth_dev_callback_unregister(port_id,
							RTE_ETH_EVENT_INTR_LSC,
							e3_lsc_callback,
							NULL);
		}
		if(pe3iface)
			RTE_FREE(pe3iface);
	error_dev_detach:
		{
			int release_rc;
			char dev_name[128];
			rte_eth_dev_stop(port_id);
			rte_eth_dev_close(port_id);
			release_rc=rte_eth_dev_detach(port_id,dev_name);
			if(release_rc)
				E3_LOG("error occurs during releasing %s \n",dev_name);
		}
		return -1;
}
void _unregister_e3interface_rcu_callback(struct rcu_head * rcu)
{
	int idx=0;
	char dev_name[128];
	int rc;
	struct E3Interface * pe3iface=container_of(rcu,struct E3Interface,rcu);
	struct node * pinput_nodes[MAX_QUEUES_TO_POLL];
	struct node * poutput_nodes[MAX_QUEUES_TO_POLL];
	if(pe3iface->interface_delete)
		pe3iface->interface_delete(pe3iface);
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		pinput_nodes[idx]=find_node_by_index(pe3iface->input_node[idx]);
		poutput_nodes[idx]=find_node_by_index(pe3iface->output_node[idx]);
		E3_ASSERT(pinput_nodes[idx]);
		E3_ASSERT(poutput_nodes[idx]);
	}
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		unregister_node(pinput_nodes[idx]);
		unregister_node(poutput_nodes[idx]);
	}
	rte_eth_dev_stop(pe3iface->port_id);
	rte_eth_dev_close(pe3iface->port_id);
	rc=rte_eth_dev_detach(pe3iface->port_id,dev_name);
	if(rc)
		E3_WARN("detaching port:%s fails\n",(char*)pe3iface->name);
	E3_LOG("successfully delete interface:%s\n",(char*)pe3iface->name);
	dealloc_e3interface(pe3iface);
}
void unregister_e3interface(int port_id)
{
	int rc;
	struct node * pinput_nodes[MAX_QUEUES_TO_POLL];
	struct node * poutput_nodes[MAX_QUEUES_TO_POLL];
	int idx=0;
	struct E3Interface * pe3iface=find_e3interface_by_index(port_id);
	if(!pe3iface){
		E3_WARN("iface:%d to be released does not exist\n",port_id);
		return ;
	}
	if(pe3iface->under_releasing)
		return ;
	pe3iface->under_releasing=1;
	
	for(idx=0;idx<pe3iface->nr_queues;idx++){
		pinput_nodes[idx]=find_node_by_index(pe3iface->input_node[idx]);
		poutput_nodes[idx]=find_node_by_index(pe3iface->output_node[idx]);
		E3_ASSERT(pinput_nodes[idx]);
		E3_ASSERT(poutput_nodes[idx]);
	}

	for(idx=0;idx<pe3iface->nr_queues;idx++){
		rc=detach_node_from_lcore(pinput_nodes[idx]);
		if(rc){
			E3_WARN("errors occur during detaching node:%s from lcore:%d\n",
				(char*)pinput_nodes[idx]->name,
				pinput_nodes[idx]->lcore_id);
		}else
			put_lcore(pinput_nodes[idx]->lcore_id,1);
		
		rc=detach_node_from_lcore(poutput_nodes[idx]);
		if(rc){
			E3_WARN("errors occur during detaching node:%s from lcore:%d\n",
				(char*)poutput_nodes[idx]->name,
				poutput_nodes[idx]->lcore_id);
		}else
			put_lcore(poutput_nodes[idx]->lcore_id,1);
	}
	/*unregister LSC event right now*/
	if(pe3iface->lsc_enabled){
		rc=rte_eth_dev_callback_unregister(pe3iface->port_id,
							RTE_ETH_EVENT_INTR_LSC,
							e3_lsc_callback,
							NULL);
		if(rc)
			E3_WARN("errors occurs during unregistering LSC callback function\n");
	}
	/*not referrable any more*/
	rcu_assign_pointer(global_e3iface_array[pe3iface->port_id],NULL);
	call_rcu(&pe3iface->rcu,_unregister_e3interface_rcu_callback);
}

int correlate_e3interfaces(struct E3Interface * pif1,struct E3Interface *pif2)
{
	
	struct E3Interface * pif_tap=NULL;
	struct E3Interface * pif_phy=NULL;
	if(pif1->hwiface_model==E3IFACE_MODEL_TAP_SINGLY_QUEUE)
		pif_tap=pif1;
	else
		pif_phy=pif1;
	if(pif2->hwiface_model==E3IFACE_MODEL_TAP_SINGLY_QUEUE)
		pif_tap=pif2;
	else
		pif_phy=pif2;
	
	if(!pif_tap || !pif_phy)
		return -1;
	if(pif_tap->has_phy_device)
		return -1;
	if(pif_phy->has_tap_device)
		return -1;
	pif_tap->peer_port_id=pif_phy->port_id;
	pif_phy->peer_port_id=pif_tap->port_id;
	__sync_synchronize();
	pif_tap->has_phy_device=1;
	pif_phy->has_tap_device=1;
	_mm_sfence();
	
	E3_LOG("correlate device:%s with tap device:%s\n",
		(char*)pif_phy->name,
		(char*)pif_tap->name);
	return 0;
}
int dissociate_e3interface(struct E3Interface * pif)
{
	struct E3Interface * pcorresponding_if=NULL;
	if(!pif->has_peer_device)
		return -1;
	pcorresponding_if=find_e3interface_by_index(pif->peer_port_id);
	if(!pcorresponding_if)
		return -2;
	if(!pcorresponding_if->has_peer_device)
		return -3;
	if(pcorresponding_if->peer_port_id!=pif->port_id)
		return -4;
	pif->has_peer_device=0;
	pcorresponding_if->has_peer_device=0;
	__sync_synchronize();
	pif->peer_port_id=0;
	pcorresponding_if->peer_port_id=0;
	_mm_sfence();
	E3_LOG("dissociate device:%s from device:%s\n",
		(char*)pif->name,
		(char*)pcorresponding_if->name);
	
	return 0;
}
int start_e3interface(struct E3Interface * pif)
{
	int rc;
	if(!pif || pif->iface_status==E3INTERFACE_STATUS_UP)
		return -1;
	rc=rte_eth_dev_start(pif->port_id);
	if(!rc)
		set_status_up_e3iface(pif);
	E3_LOG("set E3interface %s status:up\n",(char*)pif->name);
	return rc;
}

void start_e3interface_with_slow_path(int any_port_id)
{
	struct E3Interface * pif;
	struct E3Interface * pif_peer;
	pif=find_e3interface_by_index(any_port_id);
	start_e3interface(pif);
	if(pif&&pif->has_peer_device){
		pif_peer=find_e3interface_by_index(pif->peer_port_id);
		start_e3interface(pif_peer);
	}
}

void stop_e3interface_with_slow_path(int any_port_id)
{
	struct E3Interface * pif;
	struct E3Interface * pif_peer;
	pif=find_e3interface_by_index(any_port_id);
	stop_e3interface(pif);
	if(pif&&pif->has_peer_device){
		pif_peer=find_e3interface_by_index(pif->peer_port_id);
		stop_e3interface(pif_peer);
	}
}


int stop_e3interface(struct E3Interface * pif)
{
	if(!pif || pif->iface_status==E3INTERFACE_STATUS_DOWN)
		return -1;
	rte_eth_dev_stop(pif->port_id);
	set_status_down_e3iface(pif);
	E3_LOG("set E3interface %s status:down\n",(char*)pif->name);
	return 0;
}

void dump_e3interfaces(void)
{
	struct E3Interface * pif;
	int idx=0;
	for(idx=0;idx<MAX_NUMBER_OF_E3INTERFACE;idx++){
		pif=find_e3interface_by_index(idx);
		if(!pif)
			continue;
		E3_LOG("%d: ifname:%s with %d queues\n",
				pif->port_id,
				(char*)pif->name,
				pif->nr_queues);
	}
}
__attribute__((constructor))
	void e3iface_module_init(void)
{
	int idx=0;
	for(idx=0;idx<MAX_NUMBER_OF_E3INTERFACE;idx++)
		global_e3iface_array[idx]=NULL;
}

void  dump_e3_interface_structure(void)
{
	int last_offset=0,last_size=0;
		puts("dump definition: struct E3Interface");
		dump_field(struct E3Interface,name);
		dump_field(struct E3Interface,cacheline0);
		dump_field(struct E3Interface,hwiface_model);
		dump_field(struct E3Interface,hwiface_role);
		dump_field(struct E3Interface,reserved0);
		dump_field(struct E3Interface,iface_status);
		dump_field(struct E3Interface,nr_queues);
		dump_field(struct E3Interface,under_releasing);
		dump_field(struct E3Interface,has_peer_device);
		dump_field(struct E3Interface,lsc_enabled);
		dump_field(struct E3Interface,port_id);
		dump_field(struct E3Interface,peer_port_id);
		dump_field(struct E3Interface,mac_addrs);
		dump_field(struct E3Interface,input_node);
		dump_field(struct E3Interface,output_node);
		dump_field(struct E3Interface,rcu);
		dump_field(struct E3Interface,interface_up);
		dump_field(struct E3Interface,interface_down);
		dump_field(struct E3Interface,interface_delete);
		dump_field(struct E3Interface,private);
}
/*export e3interface api*/
e3_type list_e3interfaces(e3_type service,
			e3_type nr_ifaces, /*output: uint64_t* */
			e3_type ifaces /*output:uint16_t * MAX_NUMBER_OF_E3INTERFACE*/)
{
	uint64_t * _nr_ifaces=(uint64_t *)e3_type_to_uint8_t_ptr(nr_ifaces);
	uint16_t * _ifaces=(uint16_t*)e3_type_to_uint8_t_ptr(ifaces);
	struct E3Interface * pif=NULL;
	int iptr=0;
	int idx=0;
	for(idx=0;idx<MAX_NUMBER_OF_E3INTERFACE;idx++){
		pif=find_e3interface_by_index(idx);
		if(!pif)
			continue;
		_ifaces[iptr++]=idx;
	}
	*_nr_ifaces=iptr;
	return 0;
}

DECLARE_E3_API(list_e3ifaces)={
	.api_name="list_e3interfaces",
	.api_desc="enumerate e3 interfaces, return the list of available index",
	.api_callback_func=(api_callback_func)list_e3interfaces,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=8},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(uint16_t)*MAX_NUMBER_OF_E3INTERFACE},
		{.type=e3_arg_type_none},
	},
};

e3_type get_e3interface(e3_type service,
	e3_type iface_idx,/*input:the target iface index*/
	e3_type pe3iface/*output:the struct E3interface object memory*/)
{
	int _iface_idx=(int)e3_type_to_uint8_t(iface_idx);
	struct E3Interface * _pe3iface=(struct E3Interface*)e3_type_to_uint8_t_ptr(pe3iface);
	struct E3Interface * ptr=find_e3interface_by_index(_iface_idx);
	if(!ptr)
		return -1;
	memcpy(_pe3iface,ptr,sizeof(struct E3Interface));
	return 0;
}

DECLARE_E3_API(get_e3iface)={
	.api_name="get_e3interface",
	.api_desc="get the e3interface of a given index,this will copy the structure",
	.api_callback_func=(api_callback_func)get_e3interface,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct E3Interface)},
		{.type=e3_arg_type_none},
	},
	
};