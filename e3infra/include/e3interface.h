/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _E3INTERFACE_H
#define _E3INTERFACE_H
#include <inttypes.h>
#include <rte_ethdev.h>
#include <e3infra/include/e3-log.h>
#include <e3infra/include/util.h>
#include <urcu-qsbr.h>

#define MAX_E3INTERFACE_NAME_SIZE 64
#if 0
enum e3_hwiface_model{
	e3_hwiface_model_none,
	e3_hwiface_model_vlink,
	e3_hwiface_model_tap,
	e3_hwiface_model_virtio,
	e3_hwiface_model_intel_xl710,
	e3_hwiface_model_intel_x710,
	e3_hwiface_model_intel_82599,
	
};
#endif

#define E3INTERFACE_STATUS_UP 0x1
#define E3INTERFACE_STATUS_DOWN 0x0
#define MAX_QUEUES_TO_POLL 8


struct E3Interface{

	/*cacheline 0*/
	uint8_t name[MAX_E3INTERFACE_NAME_SIZE];
__attribute__((aligned(64))) 
		uint64_t cacheline0[0];/*frequently accessed fields*/
	uint8_t  hwiface_model;
	uint8_t  hwiface_role;
	uint8_t  reserved0;
	uint8_t  iface_status;/*initially set to E3INTERFACE_STATUS_DOWN,
							 lcore must stop polling when it's down,because
							 I found it can crash when application starts up
							 with burst traffic already on the wire*/
	uint8_t  nr_queues;
	uint8_t  under_releasing;
	union{
		uint8_t  has_peer_device;
		uint8_t  has_phy_device;
		uint8_t  has_tap_device;/*indicate whether 
							  	 it has corresponding tap devide*/
	};
	uint8_t lsc_enabled;   /*whether this interface is able to check LSC*/
	
	uint16_t port_id;
	uint16_t peer_port_id;
	uint8_t  mac_addrs[6];
	
	uint16_t input_node[MAX_QUEUES_TO_POLL];
	uint16_t output_node[MAX_QUEUES_TO_POLL];
	struct rcu_head rcu;
	int (*interface_up)(int iface);
	int (*interface_down)(int iface);
	/*
	*interface delete callback function
	*this is be invoked in RCU context
	*/
	int (*interface_delete)(struct E3Interface*);
__attribute__((aligned(64)))
			   void * private[0];
}__attribute__((packed));

/*
C-definition
                name (offset:  0 size: 64 prev_gap:0)
          cacheline0 (offset: 64 size:  0 prev_gap:0)
       hwiface_model (offset: 64 size:  1 prev_gap:0)
        hwiface_role (offset: 65 size:  1 prev_gap:0)
           reserved0 (offset: 66 size:  1 prev_gap:0)
        iface_status (offset: 67 size:  1 prev_gap:0)
           nr_queues (offset: 68 size:  1 prev_gap:0)
     under_releasing (offset: 69 size:  1 prev_gap:0)
     has_peer_device (offset: 70 size:  1 prev_gap:0)
         lsc_enabled (offset: 71 size:  1 prev_gap:0)
             port_id (offset: 72 size:  2 prev_gap:0)
        peer_port_id (offset: 74 size:  2 prev_gap:0)
           mac_addrs (offset: 76 size:  6 prev_gap:0)
          input_node (offset: 82 size: 16 prev_gap:0)
         output_node (offset: 98 size: 16 prev_gap:0)
                 rcu (offset:114 size: 16 prev_gap:0)
        interface_up (offset:130 size:  8 prev_gap:0)
      interface_down (offset:138 size:  8 prev_gap:0)
    interface_delete (offset:146 size:  8 prev_gap:0)
             private (offset:192 size:  0 prev_gap:38)
		
Python-definition:
		   name:<Field type=c_char_Array_64, ofs=0, size=64>
  hwiface_model:<Field type=c_ubyte, ofs=64, size=1>
   hwiface_role:<Field type=c_ubyte, ofs=65, size=1>
	  reserved0:<Field type=c_ubyte, ofs=66, size=1>
   iface_status:<Field type=c_ubyte, ofs=67, size=1>
	  nr_queues:<Field type=c_ubyte, ofs=68, size=1>
under_releasing:<Field type=c_ubyte, ofs=69, size=1>
has_peer_device:<Field type=c_ubyte, ofs=70, size=1>
	lsc_enabled:<Field type=c_ubyte, ofs=71, size=1>
		port_id:<Field type=c_ushort, ofs=72, size=2>
   peer_port_id:<Field type=c_ushort, ofs=74, size=2>
	  mac_addrs:<Field type=c_ubyte_Array_6, ofs=76, size=6>
	 input_node:<Field type=c_ushort_Array_8, ofs=82, size=16>
	output_node:<Field type=c_ushort_Array_8, ofs=98, size=16>
		_dummy0:<Field type=c_ubyte_Array_78, ofs=114, size=78>

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
	int (*post_setup)(struct E3Interface * iface);/*optional,usually the interface role will be set*/
	
    int (*input_node_process_func)(void * arg);
	int (*output_node_process_func)(void * arg);
	int (*lsc_iface_up)(int iface);
	int (*lsc_iface_down)(int iface);
	int (*iface_delete)(struct E3Interface*);
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

void dump_e3interfaces(void);

int correlate_e3interfaces(struct E3Interface * pif1,struct E3Interface *pif2);
int dissociate_e3interface(struct E3Interface * pif);
int start_e3interface(struct E3Interface * pif);
int stop_e3interface(struct E3Interface * pif);
void start_e3interface_with_slow_path(int any_port_id);
void stop_e3interface_with_slow_path(int any_port_id);



#endif
