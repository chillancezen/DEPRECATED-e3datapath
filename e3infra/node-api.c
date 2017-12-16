#include <e3infra/include/node.h>
#include <e3infra/include/util.h>
#include <e3api/include/e3-api-wrapper.h>
#include <rte_memcpy.h>


/*
*read-only API,
*no guard is taken to protect
*/
e3_type infra_api_get_node(e3_type e3api_service,
	e3_type node_index,
	e3_type pnode)
{
	int16_t _node_index=e3_type_to_uint16_t(node_index);
	struct node * _pnode=(struct node *)e3_type_to_uint8_t_ptr(pnode);
	struct node * _node=NULL;
	_node=find_node_by_index(_node_index);
	if(!_node)
		return -E3_ERR_NOT_FOUND;
	rte_memcpy(_pnode,_node,sizeof(struct node));
	return E3_OK;
}
DECLARE_E3_API(node_retrieval)={
	.api_name="infra_api_get_node",
	.api_desc="retrieve a node",
	.api_callback_func=(api_callback_func)infra_api_get_node,
	.args_desc={
		{.type=e3_arg_type_uint16_t,.behavior=e3_arg_behavior_input,},
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=sizeof(struct node)},
		{.type=e3_arg_type_none,},
	},
};


e3_type infra_api_list_nodes(e3_type e3api_service,
	e3_type entries)
{
	int8_t * _entries=(int8_t*)e3_type_to_uint8_t_ptr(entries);
	int idx=0;
	for(idx=0;idx<MAX_NR_NODES;idx++){
		if(find_node_by_index(idx))
			_entries[idx]=1;
		else
			_entries[idx]=0;
	}
	dump_nodes();
	return E3_OK;
}
DECLARE_E3_API(nodes_enumeration)={
	.api_name="infra_api_list_nodes",
	.api_desc="enumerate the nodes of all lcores",
	.api_callback_func=(api_callback_func)infra_api_list_nodes,
	.args_desc={
		{.type=e3_arg_type_uint8_t_ptr,.behavior=e3_arg_behavior_output,.len=MAX_NR_NODES},
		{.type=e3_arg_type_none,},
	},
};

void  dump_node_structure(void)
{
	int last_offset=0,last_size=0;
		puts("dump definition: struct node");
		dump_field(struct node,name);
		dump_field(struct node,nr_rx_packets);
		dump_field(struct node,nr_tx_packets);
		dump_field(struct node,nr_drop_packets);
		dump_field(struct node,node_index);
		dump_field(struct node,lcore_id);
		dump_field(struct node,burst_size);
		dump_field(struct node,node_ring);
		dump_field(struct node,next);
		dump_field(struct node,node_priv);
		dump_field(struct node,node_type);
}

