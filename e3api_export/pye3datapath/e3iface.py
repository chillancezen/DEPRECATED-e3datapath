#! /usr/bin/python3
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint
class E3Interface(Structure):
    _pack_=1
    _fields_=[('name',c_char*64),
                ('hwiface_model',c_uint32,4),
                ('iface_status',c_uint32,1),
                ('nr_queues',c_uint32,3),
                ('under_releasing',c_uint32,1),
                ('has_peer_device',c_uint32,1),
                ('lsc_enabled',c_uint32,1),
                ('port_id',c_uint16),
                ('peer_port_id',c_uint16),
                ('mac_addrs',c_uint8*6),
                ('input_node',c_uint16*8),
                ('output_node',c_uint16*8),
                ('dummy0',c_uint8*82)]
    def __str__(self):
        ret=dict()
        for item  in self._fields_:
            ret[item[0]]=getattr(self,item[0])
        #ret['raw']=repr(string_at(addressof(self),sizeof(self)))
        return repr(ret)

                
#return the list() of e3datapath interfaces 

def get_e3iface(iface):
    api_ret=c_uint64(0);
    iface_to_pass=c_uint16(iface)
    e3iface=E3Interface() 
    rc=clib.get_e3interface(byref(api_ret),iface_to_pass,byref(e3iface))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    return e3iface

def get_e3iface_list():
    class A(Structure):
        _fields_=[('a',c_uint16*256)]
    api_ret=c_uint64(0);
    nr_ifaces=c_uint64(0)
    list_iface=list()
    a=A()
    rc=clib.list_e3interfaces(byref(api_ret),byref(nr_ifaces),byref(a))
    if rc !=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    for i in range(nr_ifaces.value):
        list_iface.append(a.a[i])
    return list_iface
    
if __name__=='__main__':
    register_service_endpoint('tcp://localhost:507')
    if_lst=get_e3iface_list()
    print('interface index list:',if_lst)
    for ifidx in if_lst:
        print(get_e3iface(ifidx))
    pass
