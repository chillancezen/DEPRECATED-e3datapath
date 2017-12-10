#! /usr/sbin/python3
from pye3datapath.e3client import *
from pye3datapath.e3iface import *
from pye3datapath.common.nexthop import *
from pye3datapath.common.neighbor import *
from pye3datapath.spine.pbport import *
from pye3datapath.spine.mnexthop import *

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    print('register a provider packbone port:',attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT,True))
    print('start port:',start_e3iface(0))
    print('register a neighbor:',register_neighbor('2.2.2.2','08:00:27:53:9d:44'))
    print('register a nexthop:',register_nexthop(0,0))
    print('register label:',register_spine_label_entry(0,925516,1,1234,0)) 
    print('register multicast nexthops:',register_mnexthop())

    #register_nexthop_in_mnexthops(0,0,1111)
    #register_nexthop_in_mnexthops(0,0,1112)
    #register_spine_label_entry(0,925516,0,1112,0)

    for iface in get_e3iface_list():
        print(get_e3iface(iface))
    for l in list_spine_label_entry(0):
        print(l)
    for n in list_nexthops():
        print(n)
    for n in list_neighbors():
        print(n)
    for m in list_mnexthops():
        print(get_mnexthop(m))
