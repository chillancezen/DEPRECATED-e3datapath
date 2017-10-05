#include <e3-api-wrapper.h>
#include <exported-api.h>
#include <assert.h>
#include <stdio.h>
#include <urcu-qsbr.h>


struct E3Interface{

	/*cacheline 0*/
	uint8_t name[64];
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
	
	uint16_t input_node[8];
	uint16_t output_node[8];
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

int main()
{
	uint64_t version=0,rc;
	struct E3Interface iface;
	struct e3_api_client * client;
	client=allocate_e3_api_client("tcp://localhost:507");
	assert(client);
	publish_e3_api_client(client);

	{/*get the version number*/
		rc=e3datapath_version(&version);
		printf("e3datapath_version:%lld,%llx\n",rc,version);
	}

	{/*fetch the interface index list and fetch single element*/
		uint64_t nr_ifaces,dummy;
		uint16_t ifaces[256];
		int idx;
		rc=list_e3interfaces(&dummy,(uint8_t*)&nr_ifaces,(uint8_t*)ifaces);
		printf("list_e3interfaces:%lld\n",rc);
		for(idx=0;idx<nr_ifaces;idx++){
			rc=get_e3interface(&dummy,ifaces[idx],(uint8_t*)&iface);
			printf("fetch iface:%d api-call:%lld api-ret:%lld \n",ifaces[idx],rc,dummy);
			printf("\tname:%s\n",(char*)iface.name);
			printf("\tmac_addrs: %02x:%02x:%02x:%02x:%02x:%02x\n",iface.mac_addrs[0],
				iface.mac_addrs[1],
				iface.mac_addrs[2],
				iface.mac_addrs[3],
				iface.mac_addrs[4],
				iface.mac_addrs[5]);
			printf("\tpeer_port_id:%d\n",iface.peer_port_id);
			

		}
		//	printf("\tif:%d\n",ifaces[idx]);
	}
	
	return 0;
}
