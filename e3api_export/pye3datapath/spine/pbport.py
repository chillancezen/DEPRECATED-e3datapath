#! /usr/bin/python3
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

class Labelentry(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint32,1),
                ('is_unicast',c_uint32,1),
                ('swapped_label',c_uint32,20),
                ('reserved0',c_uint32,10),
                ('NHLFE',c_uint32)]
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
        l               =Labelentry()
        l.is_valid      =self.is_valid
        l.is_unicast    =self.is_unicast
        l.swapped_label =self.swapped_label
        l.NHLFE         =self.NHLFE
        l.index         =self.index
        l.reserved0     =self.reserved0
        return l
    def dump_definition(self):
        print(Labelentry.is_valid)
        print(Labelentry.is_unicast)
        print(Labelentry.swapped_label)
        print(Labelentry.reserved0)
        print(Labelentry.NHLFE)

def register_label_entry(iface,
                        label_index,
                        is_unicast,
                        label_to_swap,#also as RPF check label
                        nhlfe):
    api_ret     =c_uint64(0)
    _iface      =c_uint16(iface)
    _label_index=c_uint32(label_index) 
    label       =Labelentry()
    label.is_valid=1
    label.is_unicast=is_unicast
    label.swapped_label=label_to_swap
    label.NHLFE=nhlfe

    rc=clib.register_label_entry(byref(api_ret),_iface,_label_index,byref(label))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
def get_label_entry(iface,label_index):
    api_ret     =c_uint64(0)
    _iface      =c_uint16(iface)
    _label_index=c_uint32(label_index)
    label=Labelentry()
    rc=clib.get_label_entry(byref(api_ret),_iface,_label_index,byref(label))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    label.index=label_index
    return label
def list_label_entry(iface):
    nr_entries_to_fetch=256
    api_ret=c_uint64(0)
    _iface=c_uint16(iface)
    index_to_start=c_uint32(0)
    nr_entries    =c_uint32(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',Labelentry*nr_entries_to_fetch)]
    class B(Structure):
        _pack_=1
        _fields_=[('b',c_uint32*nr_entries_to_fetch)]
    a=A()
    b=B()
    lst=list()
    while True:
        rc=clib.list_label_entry_partial(byref(api_ret),
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
def delete_label_entry(iface,label_index):
    api_ret=c_uint64(0)
    _iface=c_uint16(iface)
    _lable_index=c_uint32(label_index)
    rc=clib.delete_label_entry(byref(api_ret),_iface,_lable_index)
    if rc!=0:
        raise api_call_exception()
    
if __name__=='__main__':
    Labelentry().dump_definition()
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    from pye3datapath.e3iface import attach_e3iface
    from pye3datapath.e3iface import get_e3iface_list
    from pye3datapath.e3iface import E3IFACE_MODEL_GENERIC_SINGLY_QUEUE
    from pye3datapath.e3iface import E3IFACE_ROLE_PROVIDER_BACKBONE_PORT
    from pye3datapath.e3iface import get_e3iface
    attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT)
    for iface in get_e3iface_list():
        print(get_e3iface(iface)) 
    register_label_entry(0,0x100000-1,1,0x34ef2,102) 
    register_label_entry(0,0xff03,1,0x34ef,102)
    #print(get_label_entry(0,0xff03))
    #print(get_label_entry(0,0x1))
    #print(get_label_entry(0,0xfffff))
    delete_label_entry(0,0x100-1)
    for l in list_label_entry(0):
        print(l)


