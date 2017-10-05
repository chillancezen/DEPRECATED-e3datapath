#ifndef _E3INTERFACE_H
#define _E3INTERFACE_H
#include <inttypes.h>
#include <rte_ethdev.h>
#include <e3_log.h>
#include <util.h>
#include <urcu-qsbr.h>

#define MAX_E3INTERFACE_NAME_SIZE 64
enum e3_hwiface_model{
	e3_hwiface_model_none,
	e3_hwiface_model_vlink,
	e3_hwiface_model_tap,
	e3_hwiface_model_virtio,
	e3_hwiface_model_intel_xl710,
	e3_hwiface_model_intel_x710,
	e3_hwiface_model_intel_82599,
	
};
#define E3INTERFACE_STATUS_UP 0x1
#define E3INTERFACE_STATUS_DOWN 0x0
#define MAX_QUEUES_TO_POLL 8


struct E3Interface{

	/*cacheline 0*/
	uint8_t name[MAX_E3INTERFACE_NAME_SIZE];
__attribute__((aligned(64))) 
		uint64_t cacheline0[0];/*frequently accessed fields*/
	uint8_t  hwiface_model:4;
	uint8_t  iface_status:1;/*initially set to E3INTERFACE_STATUS_DOWN,
							 lcore must stop polling when it's down,because
							 I found it can crash when application starts up
							 with burst traffic already on the wire*/
	uint8_t  nr_queues:3;
	uint8_t  under_releasing:1;
	union{
		uint8_t  has_peer_device:1;
		uint8_t  has_phy_device:1;
		uint8_t  has_tap_device:1;/*indicate whether 
							  	 it has corresponding tap devide*/
	};
	uint8_t lsc_enabled:1;   /*whether this interface is able to check LSC*/
	
	uint16_t port_id;
	uint16_t peer_port_id;
	uint8_t  mac_addrs[6];
	
	uint16_t input_node[MAX_QUEUES_TO_POLL];
	uint16_t output_node[MAX_QUEUES_TO_POLL];
	struct rcu_head rcu;
	int (*interface_up)(int iface);
	int (*interface_down)(int iface);
	
__attribute__((aligned(64)))
			   void * private[0];
}__attribute__((aligned(1)));

/*
		  name (offset:  0 size: 64 prev_gap:0)
	cacheline0 (offset: 64 size:  0 prev_gap:0)
	   port_id (offset: 68 size:  2 prev_gap:4)
  peer_port_id (offset: 70 size:  2 prev_gap:0)
	 mac_addrs (offset: 72 size:  6 prev_gap:0)
	input_node (offset: 78 size: 16 prev_gap:0)
   output_node (offset: 94 size: 16 prev_gap:0)
		   rcu (offset:112 size: 16 prev_gap:2)
  interface_up (offset:128 size:  8 prev_gap:0)
interface_down (offset:136 size:  8 prev_gap:0)
	   private (offset:192 size:  0 prev_gap:48)

*/
#define E3IFACE_PRIV(iface) ((iface)->private)
struct next_edge_item{
	int edge_entry;/*-1 indicates the end of the entries*/
	uint16_t fwd_behavior;
	char * next_ref;
};
#define MAX_PREDEFINED_EDGE 8

struct E3Interface_ops{
	int check_lsc;    /*whether to change link status chanege update*/
	int numa_socket_id;
	int priv_size;
	int (*capability_check)(int port_id);/*check the offload ability 
										  of the target port,0 indicates 
										  success.*/
	int (*queue_setup)(struct E3Interface * iface);/*model specific setup,
										  called during registeration,
										  this emulates the proceedure the Linux netdevice does*/
	int (*port_setup)(struct E3Interface * iface,
					struct rte_eth_conf * port_conf);/*mandatory provided to configure the port*/
	int (*post_setup)(struct E3Interface * iface);/*optional*/
	
    int (*input_node_process_func)(void * arg);
	int (*output_node_process_func)(void * arg);
	int (*lsc_iface_up)(int iface);
	int (*lsc_iface_down)(int iface);
	struct next_edge_item edges[MAX_PREDEFINED_EDGE];
};

extern struct E3Interface * global_e3iface_array[MAX_NUMBER_OF_E3INTERFACE];

#define find_e3interface_by_index(idx) ({\
	struct E3Interface * _pif=NULL; \
	if(((idx)<MAX_NUMBER_OF_E3INTERFACE)&&((idx)>=0)) \
		_pif=rcu_dereference(global_e3iface_array[(idx)]); \
	_pif; \
})

struct E3Interface * alloc_e3interface(int priv_size,int socket_id);
int register_e3interface(const char * params,struct E3Interface_ops * dev_ops,int *pport_id);
void unregister_e3interface(int port_id);


#define set_status_down_e3iface(pif) {\
	(pif)->iface_status=E3INTERFACE_STATUS_DOWN; \
	_mm_sfence(); \
}

#define set_status_up_e3iface(pif) {\
	(pif)->iface_status=E3INTERFACE_STATUS_UP; \
	_mm_sfence(); \
}

void dump_e3interfaces(FILE* fp);

int correlate_e3interfaces(struct E3Interface * pif1,struct E3Interface *pif2);
int dissociate_e3interface(struct E3Interface * pif);
int start_e3interface(struct E3Interface * pif);
int stop_e3interface(struct E3Interface * pif);
void start_e3interface_with_slow_path(int any_port_id);
void stop_e3interface_with_slow_path(int any_port_id);



#endif
