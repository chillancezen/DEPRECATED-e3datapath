#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint




def attach_csprt_to_eline(iface,vlan,eline):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _vlan=c_int16(vlan)
    _eline=c_int16(eline)
    rc=clib.leaf_api_csp_setup_port(byref(api_ret),_iface,_vlan,1,_eline)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to set up the port with api_ret:%x'%(api_ret.value))

if __name__=='__main__':
    from pye3datapath.e3iface import *
    from pye3datapath.common.neighbor import *
    from pye3datapath.common.nexthop import *
    from pye3datapath.leaf.etherline import *
    from pye3datapath.leaf.etherlan import *

    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    register_neighbor('130.140.150.1','08:00:27:ab:24:62')
    register_nexthop(0,0)

    print(register_ether_line_service())

    print(attach_e3iface('0000:02:05.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,True))
     
    attach_csprt_to_eline(0,100,0)
    
    print(get_ether_line_service(0))
