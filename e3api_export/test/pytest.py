#! /usr/bin/python3

from pye3datapath.e3client import *
from pye3datapath.e3backbone import *
from pye3datapath.e3iface import *
from pye3datapath.e3nhlfe import *


if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    #interface initialization
    reclaim_e3iface(0)
    print('register interface:',attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT))
    print('dump interfaces:')
    for ifidx in get_e3iface_list():
        start_e3iface(ifidx)
        print(get_e3iface(ifidx))
    #neighbour initialization
    delete_neighbour('130.140.150.1')
    delete_neighbour('130.140.150.2')
    delete_neighbour('130.140.150.3')
    register_neighbour('130.140.150.1','0:01:23:24:35:45')
    register_neighbour('130.140.150.2','0:01:23:24:35:46')
    register_neighbour('130.140.150.3','0:01:23:24:35:47')
    for n in list_neighbours():
        print(n)
    #next hop initialization
    for n in list_nexthops():
        delete_nexthop(n.index)
    register_nexthop(0,'130.140.150.1')
    register_nexthop(0,'130.140.150.2')
    register_nexthop(0,'130.140.150.3')
    for n in list_nexthops():
        print(n)
    #multicast hop initialization
    for n in list_multicast_nexthops():
        delete_multicast_nexthops(n.index)
    register_multicast_nexthops([1,2])
    for n in list_multicast_nexthops():
        print(n)
    #register label
    register_label_entry(0,925516,1,0x520,1)
    for l in list_label_entry(0):
        print(l) 
