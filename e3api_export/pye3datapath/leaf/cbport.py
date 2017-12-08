#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint


class leaf_label_entry(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint8,1),
                ('egress_nhlfe_index',c_uint8,7),
                ('e3_service',c_uint8),
                ('service_index',c_int16)]
    index=0
    def __str__(self):
        ret=dict()
        ret['index']=self.index
        ret['is_valid']=self.is_valid
        ret['egress_nhlfe_index']=self.egress_nhlfe_index
        ret['e3_service']=self.e3_service
        ret['service_index']=self.service_index
        return str(ret)
    def clone(self):
        l=leaf_label_entry()
        l.index=self.index
        l.is_valid=self.is_valid
        l.egress_nhlfe_index=self.egress_nhlfe_index
        l.e3_service=self.e3_service
        l.service_index=self.service_index
        return l
    def dump_definition(self):
        print('size of leaf_label_entry:',sizeof(leaf_label_entry))
        print(leaf_label_entry.is_valid,'is_valid')
        print(leaf_label_entry.egress_nhlfe_index,'egress_nhlfe_index')
        print(leaf_label_entry.e3_service,'e3_service')
        print(leaf_label_entry.service_index,'service_index')

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
def get_cbport_label_entry(iface,label):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _label=c_int32(label)
    l=leaf_label_entry()
    rc=clib.leaf_api_cbp_get_label_entry(byref(api_ret),_iface,_label,byref(l))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('can not fetch label of a customer backbone port with api_ret:%x'%(api_ret.value))
    l.index=label
    return l

CBP_MAX_NR_ENTRIES_PER_FETCH=1024
def list_cbport_label_entries(iface):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    index_to_start=c_int32(0)
    nr_entries=c_int16(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',leaf_label_entry*CBP_MAX_NR_ENTRIES_PER_FETCH)]
    class B(Structure):
        _pack_=1
        _fields_=[('b',c_int32*CBP_MAX_NR_ENTRIES_PER_FETCH)]
    a=A()
    b=B()
    lst=list()
    while True:
        rc=clib.leaf_api_cbp_list_label_entry_partial(byref(api_ret),_iface,byref(index_to_start),byref(nr_entries),byref(a),byref(b))
        if rc!=0:
            raise api_call_exception()
        if api_ret.value!=0:
            raise api_return_exception('not a valid customer backbone port with api_ret:%x'%(api_ret.value))
        for i in range(nr_entries.value):
            l=a.a[i].clone()
            l.index=b.b[i]
            lst.append(l)
        if nr_entries.value!=CBP_MAX_NR_ENTRIES_PER_FETCH:
            break
    return lst
'''
this function set the egress nhlfe index of a label entry of
a customer backbone port, it can succeed if and only if <nhlfe,label_to_push>
is already registered in E-LAN service to which such label entry has been attached.
'''
def set_cbport_set_label_entry_peer(iface,label,nhlfe,label_to_push):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _label=c_int32(label)
    _nhlfe=c_int16(nhlfe)
    _label_to_push=c_int32(label_to_push)    
    rc=clib.leaf_api_set_cbp_egress_nhlfe_index(byref(api_ret),_iface,_label,_nhlfe,_label_to_push)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to set up the egress nhlfe index of the label entry with api_ret:%x'%(api_ret.value))
def clear_cbport_label_entry_peer(iface,label):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _label=c_int32(label)
    rc=clib.leaf_api_clear_cbp_egress_nhlfe_index(byref(api_ret),_iface,_label)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        api_return_exception('failed to clean the egress nhlfe index of the label entry with api_ret:%x'%(api_ret.value))

if __name__=='__main__':
    from pye3datapath.e3iface import *
    from pye3datapath.common.neighbor import *
    from pye3datapath.common.nexthop import *
    from pye3datapath.leaf.etherline import *
    from pye3datapath.leaf.etherlan import *
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    register_neighbor('123.130.13.1','02:42:06:9a:ad:19')
    register_nexthop(0,0)
    leaf_label_entry().dump_definition()    
    print(register_ether_line_service())
    print(register_ether_lan_service())
    print(attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,True))
    attach_cbport_to_eline(0,100,0)
    attach_cbport_to_elan(0,101,0)
    attach_cbport_to_elan(0,102,0)
    register_ether_lan_nhlfe(0,0,1023)

    #detach_cbport(0,101)
    #detach_cbport(0,102)
    set_cbport_set_label_entry_peer(0,102,0,1023)
    clear_cbport_label_entry_peer(0,103)
    for eline in list_ether_line_services():
        print(get_ether_line_service(eline))
    for elan in list_ether_lan_services():
        print(get_ether_lan_service(elan))
    for l in list_cbport_label_entries(0):
        print(l)
    print(get_cbport_label_entry(0,0))
    print(get_cbport_label_entry(0,100))
    print(get_cbport_label_entry(0,101))
    print(get_cbport_label_entry(0,102))
