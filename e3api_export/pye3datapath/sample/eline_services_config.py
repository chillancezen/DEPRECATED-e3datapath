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
    register_ether_line_service()
    register_ether_line_service()

    #e-line0 and e-line1 are peers

    #LSP setup
    #attach port 0's VLAN 1000 to E-Line service
    attach_csport_to_eline(0,1000,0)
    
    #assume port 4 allocate label:1 for this e-line service
    #and port 8 allocate label:100 for the eline service
    register_ether_line_nhlfe(0,0,1)
    register_spine_label_entry(4,1,1,100,2)
    
    #at another leaf vswitch, terminate MPLS forwarding by redirect it to an e-line service
    #which is dedicated to VLAN 2000

    attach_cbport_to_eline(8,100,1)
    attach_csport_to_eline(10,2000,1)
    #till this, a singly directed path is set up.
    #now setup the reverse path

    #assume port 6 allocate label:10000 for this e-line service
    #and port 2 allocate label:10 for this e-line service
    register_ether_line_nhlfe(1,3,10000)
    register_spine_label_entry(6,10000,1,10,1)
    
    #then the leaf switch should know label:10 is directed to e-line0
    attach_cbport_to_eline(2,10,0)

      
    for eline in list_ether_line_services():
        print(get_ether_line_service(eline))
    for entry in list_cspirt_dist_table(0):
        print(entry)
    for entry in list_spine_label_entry(4):
        print(entry)
    for entry in list_cspirt_dist_table(10):
        print(entry)
