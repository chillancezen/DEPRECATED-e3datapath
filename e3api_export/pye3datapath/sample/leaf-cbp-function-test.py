#! /usr/sbin/python3
from pye3datapath.e3client import *
from pye3datapath.e3iface import *
from pye3datapath.common.nexthop import *
from pye3datapath.common.neighbor import *
from pye3datapath.leaf.etherline import *
from pye3datapath.leaf.etherlan import *
from pye3datapath.leaf.cbport import *
from pye3datapath.leaf.csport import *

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    print('register a customer packbone port:',attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,True))
    print('start port:',start_e3iface(0))
    print('register a neighbor:',register_neighbor('2.2.2.2','08:00:27:53:9d:44'))
    print('register a nexthop:',register_nexthop(0,0))
    print('register an e-line service:',register_ether_line_service())
    print('register an e-lan service:',register_ether_lan_service())
    
    attach_cbport_to_eline(0,925516,0)
    register_ether_line_nhlfe(0,0,2345)
    register_ether_line_port(0,0,507)
    register_ether_line_port(0,1,507)

    register_ether_lan_port(0,0,508)
    register_ether_lan_port(0,0,509)

    attach_cbport_to_elan(0,925516,0)
    for iface in get_e3iface_list():
        print(get_e3iface(iface))
    for n in list_nexthops():
        print(n)
    for n in list_neighbors():
        print(n)
    for eline in list_ether_line_services():
        print(get_ether_line_service(eline))
    for elan in list_ether_lan_services():
        print(get_ether_lan_service(elan))
    for l in list_cbport_label_entries(0):
        print(l)
