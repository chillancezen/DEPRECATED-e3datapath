#include <provider-backbone-port.h>
#include <e3iface-inventory.h>
#include <node.h>
#include <mbuf_delivery.h>
#include <e3_init.h>
#include <e3_log.h>

#define PBP_NODE_BURST_SIZE 48

extern struct e3iface_role_def  role_defs[E3IFACE_ROLE_MAX_ROLES];

static int null_capability_check(int port_id)
{
	return 0;
}
#define PBP_INPUT_NODE_FWD_DROP 0x0
#define PBP_INPUT_NODE_FWD_NEXT_HOP 0x1
#define PBP_INPUT_NODE_HOST_STACK 0x2

inline uint64_t _process_pbp_input_packet(struct rte_mbuf * mbuf)
{
	
}
/*receive a packet*/
int provider_backbone_port_iface_input_iface(void * arg)
{
	struct rte_mbuf *    mbufs[PBP_NODE_BURST_SIZE];
	int                  nr_rx;
	int                  iptr;
	int 				 process_rc;
	int                  start_index,
					     end_index;
	struct node *        pnode=(struct node *)arg;
	int                  iface=HIGH_UINT64((uint64_t)pnode->node_priv);
	int                  queue_id=LOW_UINT64((uint64_t)pnode->node_priv);
	struct E3Interface * pif=find_e3interface_by_index(iface);
	struct pbp_private * priv=(struct pbp_private*)pif->private;
	DEF_EXPRESS_DELIVERY_VARS();
	RESET_EXPRESS_DELIVERY_VARS();
	uint64_t fwd_id;
	uint64_t last_fwd_id;
	
	if(PREDICT_FALSE(!pif))
		return 0;
	nr_rx=rte_eth_rx_burst(iface,queue_id,mbufs,PBP_NODE_BURST_SIZE);
	pre_setup_env(nr_rx);
	while((iptr=peek_next_mbuf())>=0){
		prefetch_next_mbuf(mbufs,iptr);
		fwd_id=MAKE_UINT64(PBP_INPUT_NODE_FWD_DROP,0);
		
		process_rc=proceed_mbuf(iptr,fwd_id);
		if(process_rc==MBUF_PROCESS_RESTART){
			fetch_pending_index(start_index,end_index);
			fetch_pending_fwd_id(last_fwd_id);

			flush_pending_mbuf();
			proceed_mbuf(iptr,fwd_id);
		}
	}
	fetch_pending_index(start_index,end_index);
	fetch_pending_fwd_id(last_fwd_id);
	return 0;
}

int provider_backbone_port_iface_output_iface(void * arg)
{
	return 0;
}

int provider_backbone_port_iface_post_setup(struct E3Interface * pif)
{
	int numa_socket;
	struct pbp_private * priv=(struct pbp_private*)pif->private;
	pif->hwiface_role=E3IFACE_ROLE_PROVIDER_BACKBONE_PORT;
	/*setup the label base for the e3interface*/
	priv->label_base=allocate_label_entry_base(-1);
	E3_ASSERT(priv->label_base);
	return 0;
}


void pbp_init(void)
{
	
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].is_set=1;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].capability_check=null_capability_check;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].priv_size=sizeof(struct pbp_private);
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].input_node_process_func=provider_backbone_port_iface_input_iface;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].output_node_process_func=provider_backbone_port_iface_output_iface;
	role_defs[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT].post_setup=provider_backbone_port_iface_post_setup;
}
E3_init(pbp_init,(TASK_PTIORITY_LOW+1));

