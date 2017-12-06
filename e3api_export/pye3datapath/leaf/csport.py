#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint




def attach_csport_to_eline(iface,vlan,eline):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _vlan=c_int16(vlan)
    _eline=c_int16(eline)
    rc=clib.leaf_api_csp_setup_port(byref(api_ret),_iface,_vlan,1,_eline)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to set up the port with api_ret:%x'%(api_ret.value))
def attach_csport_to_elan(iface,vlan,elan):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _vlan=c_int16(vlan)
    _elan=c_int16(elan)
    rc=clib.leaf_api_csp_setup_port(byref(api_ret),_iface,_vlan,0,_elan)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to set up the port with api_ret:%x'%(api_ret.value))
def detach_csport(iface,vlan):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _vlan=c_int16(vlan)
    rc=clib.leaf_api_csp_withdraw_port(byref(api_ret),_iface,_vlan)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('can not withdraw port vlan entry with api_ret:%x'(api_ret.value))

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
    print(register_ether_lan_service())
    print(attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,True))
     
    attach_csport_to_eline(0,100,0)
    attach_csport_to_elan(0,100,0)
    attach_csport_to_eline(0,100,0)
    #detach_csport(0,100)
    #delete_ether_lan_service(0)
    #delete_ether_line_service(0)
    print(get_ether_line_service(0))
    get_ether_lan_service(0).tabulate()
    for iface in get_e3iface_list():
        get_e3iface(iface).tabulate()
