#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint


def attach_cbport_to_eline(iface,label,eline):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _label=c_int32(label)
    _eline=c_int16(eline)
    rc=clib.leaf_api_cbp_setup_label_entry(byref(api_ret),_iface,_label,1,_eline)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('can not set the label entry as going-to-e-line with api_ret:%x'%(api_ret.value))
def attach_cbport_to_elan(iface,label,elan):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _label=c_int32(label)
    _elan=c_int16(elan)
    rc=clib.leaf_api_cbp_setup_label_entry(byref(api_ret),_iface,_label,0,_elan)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('can not set the label entry as going-to-e-lan with api_ret:%x'%(api_ret.value))
def detach_cbport(iface,label):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _label=c_int32(label)
    rc=clib.leaf_api_cbp_clear_label_entry(byref(api_ret),_iface,_label)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('can not detach a customer backbone port\'s label entry with api_ret:%d'%(api_ret))

if __name__=='__main__':
    from pye3datapath.e3iface import *
    from pye3datapath.common.neighbor import *
    from pye3datapath.common.nexthop import *
    from pye3datapath.leaf.etherline import *
    from pye3datapath.leaf.etherlan import *
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    
    print(register_ether_line_service())
    print(register_ether_lan_service())
    print(attach_e3iface('0000:02:05.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,True))
    attach_cbport_to_eline(0,100,0)
    attach_cbport_to_elan(0,101,0)
    attach_cbport_to_elan(0,102,0)
    detach_cbport(0,101)
    detach_cbport(0,102)
    for eline in list_ether_line_services():
        print(get_ether_line_service(eline))
    for elan in list_ether_lan_services():
        print(get_ether_lan_service(elan))

