#! /usr/bin/python3
from pye3datapath.e3client import *
from pye3datapath.e3iface import *
from pye3datapath.common.nexthop import *
from pye3datapath.common.neighbor import *
from pye3datapath.leaf.etherline import *
from pye3datapath.leaf.etherlan import *
from pye3datapath.leaf.cbport import *
from pye3datapath.leaf.csport import *
from pye3datapath.spine.pbport import *
from pye3datapath.spine.mnexthop import *

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    register_ether_lan_service()
    register_ether_lan_service()
    register_mnexthop()
     
    attach_csport_to_elan(0,3000,0)
    attach_csport_to_elan(10,4000,1)

    register_nexthop_in_mnexthops(0,2,101)
    register_nexthop_in_mnexthops(0,1,11)
    
     
    #setup multicast-path
    set_ether_lan_multicast_fwd_entry(0,0,2)
    register_spine_label_entry(4,2,0,11,0)
    register_spine_label_entry(6,10001,0,101,0)
    
    attach_cbport_to_elan(8,101,1)
    attach_cbport_to_elan(2,11,0)
    
    set_ether_lan_multicast_fwd_entry(1,3,10001)
     
    #setup unicast-path
    register_ether_lan_nhlfe(0,0,3)
    register_spine_label_entry(4,3,1,102,2)
    attach_cbport_to_elan(8,102,1)
    
    register_ether_lan_nhlfe(1,3,10002)
    register_spine_label_entry(6,10002,1,12,1)
    attach_cbport_to_elan(2,12,0)
      
    for elan in list_ether_lan_services():
        get_ether_lan_service(elan).tabulate()
