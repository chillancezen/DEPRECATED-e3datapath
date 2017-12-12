#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

class spine_label_entry(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint32,1),
                ('is_unicast',c_uint32,1),
                ('swapped_label',c_uint32,20),
                ('reserved0',c_uint32,10),
                ('NHLFE',c_int32)]
    index=0
    def __str__(self):
        ret=dict()
        ret['is_valid']     =self.is_valid
        ret['is_unicast']   =self.is_unicast
        if self.is_unicast==1:
            ret['swapped_label']=self.swapped_label
        else:
            ret['rpf_check_label']=self.swapped_label
        ret['NHLFE']        =self.NHLFE
        ret['index']        =self.index
        return repr(ret)
    def clone(self):
        l               =spine_label_entry()
        l.is_valid      =self.is_valid
        l.is_unicast    =self.is_unicast
        l.swapped_label =self.swapped_label
        l.NHLFE         =self.NHLFE
        l.index         =self.index
        l.reserved0     =self.reserved0
        return l
    def dump_definition(self):
        print('size of spine_label_entry:',sizeof(spine_label_entry))
        print(spine_label_entry.is_valid)
        print(spine_label_entry.is_unicast)
        print(spine_label_entry.swapped_label)
        print(spine_label_entry.reserved0)
        print(spine_label_entry.NHLFE)

def register_spine_label_entry(iface,
                        label_index,
                        is_unicast,
                        label_to_swap,#also as RPF check label
                        nhlfe):
    api_ret     =c_int64(0)
    _iface      =c_int16(iface)
    _label_index=c_int32(label_index) 
    label       =spine_label_entry()
    label.is_valid=1
    label.is_unicast=is_unicast
    label.swapped_label=label_to_swap
    label.NHLFE=nhlfe

    rc=clib.spine_api_pbp_setup_label_entry(byref(api_ret),_iface,_label_index,byref(label))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('api_ret:%x'%(api_ret.value))
def get_spine_label_entry(iface,label_index):
    api_ret     =c_int64(0)
    _iface      =c_int16(iface)
    _label_index=c_int32(label_index)
    label=spine_label_entry()
    rc=clib.spine_api_pbp_get_label_entry(byref(api_ret),_iface,_label_index,byref(label))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    label.index=label_index
    return label

MAX_SPINE_LABEL_ENTRIES_PER_FETCH=256
def list_spine_label_entry(iface):
    nr_entries_to_fetch=MAX_SPINE_LABEL_ENTRIES_PER_FETCH
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    index_to_start=c_int32(0)
    nr_entries    =c_int32(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',spine_label_entry*nr_entries_to_fetch)]
    class B(Structure):
        _pack_=1
        _fields_=[('b',c_uint32*nr_entries_to_fetch)]
    a=A()
    b=B()
    lst=list()
    while True:
        rc=clib.cbp_api_list_spine_label_entry_partial(byref(api_ret),
                                        _iface,
                                        byref(index_to_start),
                                        byref(nr_entries),
                                        byref(a),
                                        byref(b))
        if rc!=0:
            raise api_call_exception()
        if api_ret.value!=0:
            raise api_return_exception()
        for i in range(nr_entries.value):
            l=a.a[i].clone()
            l.index=b.b[i]
            lst.append(l)
        if nr_entries.value!=nr_entries_to_fetch:
            break
    return lst

def delete_spine_label_entry(iface,label_index):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _lable_index=c_uint32(label_index)
    rc=clib.spine_api_pbp_delete_label_entry(byref(api_ret),_iface,_lable_index)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('api_ret:%x'%(api_ret.value))
def tabulate_pbport_label_entries(iface):
    table=list()
    for l in list_spine_label_entry(iface):
        is_unicast='unicast'
        if l.is_unicast==0:
            is_unicast='multicast'
        table.append([l.index,
                is_unicast,
                l.NHLFE,
                l.swapped_label])
    print(tabulate.tabulate(table,['index(label)','nexthop type','nexthop','swapped label'],tablefmt='psql'))
    
if __name__=='__main__':
    spine_label_entry().dump_definition()
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    from pye3datapath.e3iface import *
    from pye3datapath.common.nexthop import *
    from pye3datapath.common.neighbor import *
    from pye3datapath.spine.mnexthop import *
    print(register_neighbor('130.140.250.1','02:42:7e:5f:17:ee'))
    print(register_nexthop(0,0))
    print(register_mnexthop())
    attach_e3iface('0000:02:05.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)
    register_spine_label_entry(0,1000,1,234,0)
    register_spine_label_entry(0,10000,0,234,0)

    #delete_spine_label_entry(0,10000)   
    #print(get_spine_label_entry(0,1000))
    for l in list_spine_label_entry(0):
        print(l)
    #delete_spine_label_entry(0,100000)
    tabulate_pbport_label_entries(0)
    #for n in list_nexthops():
    #    print(n)
    #for iface in get_e3iface_list():
    #    print(get_e3iface(iface)) 
    #register_label_entry(0,0x100000-1,1,0x34ef2,102) 
    #register_label_entry(0,0xff03,1,0x34ef,102)
    #print(get_label_entry(0,0xff03))
    #print(get_label_entry(0,0x1))
    #print(get_label_entry(0,0xfffff))
    #delete_label_entry(0,0x100-1)
    #for l in list_label_entry(0):
    #    print(l)


