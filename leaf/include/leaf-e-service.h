/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _LEAF_E_SERVICE_H
#define _LEAF_E_SERVICE_H
#include <stdint.h>
#include <e3net/include/common-nhlfe.h>
#include <e3infra/include/fast-index.h>
#include <urcu-qsbr.h>
#include <rte_spinlock.h>
#include <rte_rwlock.h>

#define MAX_E_LINE_SERVICES 4096
#define MAX_E_LAN_SERVICES 4096


struct ether_e_line{
	/*
	*if label_to_push and NHLFE is set,is_cbp_ready will be set to 1
	*if e3iface and vlan_tci is set,is_csp_ready is set to 1
	*/
	uint8_t  is_cbp_ready:1;
	uint8_t  is_csp_ready:1;
	uint8_t  is_valid:1;
	uint8_t  reserved0;
	
	uint16_t index;
	/*
	*bacbone side fields
	*/
	uint32_t label_to_push;
	uint16_t NHLFE;
	/*
	*customer side fields
	*/
	uint16_t e3iface;
	uint16_t vlan_tci;
	
	uint16_t ref_cnt;
}__attribute__((packed));


/*
Python definition:
<Field type=c_ubyte, ofs=0:0, bits=1> :ether_eline.is_cbp_ready
<Field type=c_ubyte, ofs=0:1, bits=1> :ether_eline.is_csp_ready
<Field type=c_ubyte, ofs=0:2, bits=1> :ether_eline.is_valid
<Field type=c_ubyte, ofs=1, size=1>   :ether_eline.reserved0
<Field type=c_ushort, ofs=2, size=2>  :ether_eline.index
<Field type=c_uint, ofs=4, size=4>    :ether_eline.label_to_push
<Field type=c_ushort, ofs=8, size=2>  :ether_eline.NHLFE
<Field type=c_ushort, ofs=10, size=2> :ether_eline.e3iface
<Field type=c_ushort, ofs=12, size=2> :ether_eline.vlan_tci
<Field type=c_ushort, ofs=14, size=2> :ether_eline.ref_cnt
*/

#define MAX_PORTS_IN_E_LAN_SERVICE 64
#define MAX_NHLFE_IN_E_LAN_SERVICE 64
/*
*MAX_NHLFE_IN_E_LAN_SERVICE must be less than 128
*/

struct ether_e_lan{
	uint8_t is_valid;
	uint8_t is_releasing;
	uint16_t index;
	uint16_t ref_cnt;
	uint16_t nr_ports;
	uint16_t nr_nhlfes;
	uint16_t multicast_NHLFE;
	uint32_t multicast_label;
	struct findex_2_4_base * fib_base;
	struct{
		uint16_t is_valid;
		uint16_t iface;
		uint16_t vlan_tci;
		uint16_t reserved0;
	}ports[MAX_PORTS_IN_E_LAN_SERVICE];
	struct{
		uint16_t is_valid;
		uint16_t NHLFE;
		uint32_t label_to_push;
	}nhlfes[MAX_NHLFE_IN_E_LAN_SERVICE];
	struct rcu_head rcu;
	rte_spinlock_t per_e_lan_guard;
	/*
	*per_e_lan_guard is being used for securing e-lan's
	*inner data fields.
	*/
}__attribute__((packed));
/*
Python definition:
size of ether_lan: 1068
<Field type=c_ubyte, ofs=0, size=1> is_valid
<Field type=c_ubyte, ofs=1, size=1> is_releasing
<Field type=c_short, ofs=2, size=2> index
<Field type=c_short, ofs=4, size=2> ref_cnt
<Field type=c_short, ofs=6, size=2> nr_ports
<Field type=c_short, ofs=8, size=2> nr_nhlfes
<Field type=c_short, ofs=10, size=2> multicast_NHLFE
<Field type=c_uint, ofs=12, size=4> multicast_label
<Field type=c_ulong, ofs=16, size=8> fib_base
<Field type=port_entry_Array_64, ofs=24, size=512> ports
<Field type=nhlfe_entry_Array_64, ofs=536, size=512> nhlfes
<Field type=c_ubyte_Array_20, ofs=1048, size=20> reserved0
*/
__attribute__((always_inline))
	static inline void get_e_lan(struct ether_e_lan * elan)
{
	rte_spinlock_lock(&elan->per_e_lan_guard);
}
__attribute__((always_inline))
	static inline void put_e_lan(struct ether_e_lan * elan)
{
	rte_spinlock_unlock(&elan->per_e_lan_guard);
}

extern struct ether_e_line * e_line_base;
#define _find_e_line_service(index) ((((index)>=0)&&((index)<MAX_E_LINE_SERVICES))?&e_line_base[(index)]:NULL)
#define find_e_line_service(index) (((((index)>=0)&&((index)<MAX_E_LINE_SERVICES))&&(e_line_base[(index)].is_valid))?&e_line_base[(index)]:NULL)
int register_e_line_service(void);
int register_e_line_port(int eline_index,int e3iface,int vlan_tci);
int register_e_line_nhlfe(int eline_index,int NHLFE,int label_to_push);
int delete_e_line_port(int eline_index);
int delete_e_line_nhlfe(int eline_index);
int reference_e_line_service(int index);
int dereference_e_line_service(int index);
int delete_e_line_service(int index);

extern struct ether_e_lan  * e_lan_base;
#define _find_e_lan_service(index) ((((index)>=0)&&((index)<MAX_E_LAN_SERVICES))?&e_lan_base[(index)]:NULL)
#define find_e_lan_service(index) (((((index)>=0)&&((index)<MAX_E_LAN_SERVICES))&&(e_lan_base[(index)].is_valid))?&e_lan_base[(index)]:NULL)
int register_e_lan_service(void);
int reference_e_lan_service(int index);
int dereference_e_lan_service(int index);
int delete_e_lan_service(int index);
int register_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci);
int find_e_lan_port(int elan_index,uint16_t e3iface,uint16_t vlan_tci);
int find_e_lan_port_locked(int elan_index,uint16_t e3iface,uint16_t vlan_tci);
int delete_e_lan_port(int elan_index,int port_index);
int register_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push);
int find_e_lan_nhlfe(int elan_index,uint16_t nhlfe,uint32_t label_to_push);
int find_e_lan_nhlfe_locked(int elan_index,uint16_t nhlfe,uint32_t label_to_push);
int delete_e_lan_nhlfe(int elan_index,int nhlfe_index);

struct e_lan_fwd_entry{
	union{
		struct{
			uint16_t is_port_entry;
			union{
				uint16_t field_u16;
				uint16_t e3iface;
				uint16_t NHLFE;
			};
			union{
				uint32_t field_u32;
				uint32_t vlan_tci;
				uint32_t label_to_push;
			};
		};
		uint64_t entry_as_u64;
	};
}__attribute__((packed));

#define mac_to_findex_2_4_key(mac,key) {\
	(key)->key_index=MAKE_UINT16((mac)[3],(mac)[4]); \
	(key)->tag1=MAKE_UINT16((mac)[0],(mac)[1]); \
	(key)->tag2=MAKE_UINT16((mac)[2],(mac)[5]); \
}

#define findex_2_4_key_to_mac(key,mac) {\
    (mac)[0]=HIGH_UINT16((key)->tag1); \
    (mac)[1]=LOW_UINT16((key)->tag1); \
    (mac)[2]=HIGH_UINT16((key)->tag2); \
    (mac)[3]=HIGH_UINT16((key)->key_index); \
    (mac)[4]=LOW_UINT16((key)->key_index); \
    (mac)[5]=LOW_UINT16((key)->tag2); \
}
int register_e_lan_fwd_entry(int elan_index,uint8_t * mac,struct e_lan_fwd_entry * fwd_entry);
int delete_e_lan_fwd_entry(int elan_index,uint8_t *mac);

/*
*the criticl section which is guarded by those read locks below
*will not acquire any write locks anymore, or call any fucntion
*that acuiqures write locks
*/
void __read_lock_eline();
void __read_unlock_eline();
void __read_lock_elan();
void __read_unlock_elan();

#endif