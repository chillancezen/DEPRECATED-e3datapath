#include <e3-api-wrapper.h>
#include <exported-api.h>
#include <assert.h>
#include <stdio.h>
#include <urcu-qsbr.h>

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
	
__attribute__((aligned(64)))
			   void * private[0];
}__attribute__((aligned(1)));


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
