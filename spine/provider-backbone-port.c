#include <provider-backbone-port.h>
#include <e3iface-inventory.h>
#include <node.h>
#include <mbuf_delivery.h>
#include <e3_init.h>
#include <e3_log.h>
#include <rte_ether.h>
#include <label-fib.h>
#include <label-nhlfe.h>
#define PBP_NODE_BURST_SIZE 48

extern struct e3iface_role_def  role_defs[E3IFACE_ROLE_MAX_ROLES];

static int null_capability_check(int port_id)
{
	return 0;
}
#define PBP_INPUT_NODE_FWD_DROP 0x0
#define PBP_INPUT_NODE_FWD_NEXT_HOP 0x1
#define PBP_INPUT_NODE_HOST_STACK 0x2

#define ETHER_PROTO_MPLS_UNICAST 0x4788

struct pbp_cache_entry{
	uint8_t                       is_valid;
	uint32_t 			 		  label;/*0 indicate invalid*/
	struct label_entry 			* lentry;
	struct next_hop    			* unicast_nexthop;
	struct topological_neighbor * unicast_neighbor;
	struct multicast_next_hops  * multicast_nexthops;
};
#define PBP_CACHE_SIZE 0x8
#define PBP_CACHE_MASK (PBP_CACHE_SIZE-1)

#define PBP_PROCESS_INPUT_DROP 0x0
#define PBP_PROCESS_INPUT_UNICAST_FWD 0x1
#define PBP_PROCESS_INPUT_MULTICAST_FWD 0x2
#define PBP_PROCESS_INPUT_HOST_STACK 0x3

inline uint64_t _process_pbp_input_packet(struct rte_mbuf * mbuf,
											struct pbp_cache_entry * pbp_cache,
											struct pbp_private     * priv)
{
	uint64_t fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_DROP,0);
	uint32_t label;
	uint32_t cache_index;
	struct ether_hdr * eth_hdr =rte_pktmbuf_mtod(mbuf,struct ether_hdr*);
	struct mpls_hdr  * mpls_hdr=(struct mpls_hdr*)(eth_hdr+1);
	if(PREDICT_FALSE(eth_hdr->ether_type!=ETHER_PROTO_MPLS_UNICAST)){
		/*
		*any non-MPLS packet goes to host stack
		*/
		fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_HOST_STACK,0);
		goto ret;
	}
	label=mpls_label(mpls_hdr);
	cache_index=label&PBP_CACHE_MASK;
	{
		/*
		*process pbp cache relavant affairs.
		*/
		if(PREDICT_FALSE(pbp_cache[cache_index].label!=label)){
			pbp_cache[cache_index].is_valid=0;
			pbp_cache[cache_index].label=label;
			pbp_cache[cache_index].lentry=label_entry_at(priv->label_base,label);
			if(!pbp_cache[cache_index].lentry->is_valid)
				goto normal;
			if(pbp_cache[cache_index].lentry->is_unicast){
				pbp_cache[cache_index].unicast_nexthop=
					next_hop_at(pbp_cache[cache_index].lentry->NHLFE);
				if(!pbp_cache[cache_index].unicast_nexthop->is_valid)
					goto normal;
				pbp_cache[cache_index].unicast_neighbor=
					topological_neighbour_at(pbp_cache[cache_index].unicast_nexthop->remote_neighbor_index);
				if(!pbp_cache[cache_index].unicast_neighbor->is_valid)
					goto normal;
			}else{
				pbp_cache[cache_index].multicast_nexthops=
					mnext_hops_at(pbp_cache[cache_index].lentry->NHLFE);
				if(!pbp_cache[cache_index].multicast_nexthops->is_valid)
					goto normal;
			}
			pbp_cache[cache_index].is_valid=1;
		}
		normal:
		/*
		*drop any packet whose fwd entries are incomplete
		*/
		if(PREDICT_FALSE(!pbp_cache[cache_index].is_valid)){
			fwd_id=MAKE_UINT64(PBP_PROCESS_INPUT_DROP,0);
			goto ret;
		}
		printf("valid mpls:%x\n",label);
	}
	ret:
	return fwd_id;	
}

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
	struct pbp_cache_entry pbp_cache[PBP_CACHE_SIZE];
	memset(pbp_cache,0x0,sizeof(struct pbp_cache_entry));
	if(PREDICT_FALSE(!pif))
		return 0;
	nr_rx=rte_eth_rx_burst(iface,queue_id,mbufs,PBP_NODE_BURST_SIZE);
	pre_setup_env(nr_rx);
	while((iptr=peek_next_mbuf())>=0){
		prefetch_next_mbuf(mbufs,iptr);
		fwd_id=MAKE_UINT64(PBP_INPUT_NODE_FWD_DROP,0);
		fwd_id=_process_pbp_input_packet(mbufs[iptr],pbp_cache,priv);
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

