#! /usr/bin/python3
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
    attach_e3iface('0000:00:05.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,True)
    attach_e3iface('0000:00:06.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,True)
    attach_e3iface('0000:00:07.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT,True)
    attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT,True)
    attach_e3iface('0000:00:09.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,True)
    attach_e3iface('0000:00:0a.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,True)
    print('enumerate e3interfaces:')
    if_lst=get_e3iface_list()
    for ifidx in if_lst:
        start_e3iface(ifidx)
        print(get_e3iface(ifidx))
    
    register_neighbor('169.254.169.2','52:53:54:55:56:02')
    register_neighbor('169.254.169.3','52:53:54:55:56:03')
    register_neighbor('169.254.169.4','52:53:54:55:56:04')
    register_neighbor('169.254.169.5','52:53:54:55:56:05')
    print('enumerate neighbors:')
    for n in list_neighbors():
        print(n)
    
    register_nexthop(2,1)
    register_nexthop(4,0)
    register_nexthop(6,3)
    register_nexthop(8,2)
    print('enumerate nexthops:')
    for n in list_nexthops():
        print(n)
    
