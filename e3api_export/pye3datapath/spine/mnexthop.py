#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

MAX_MULTICAST_NEXT_HOPS=2048
MAX_HOPS_IN_MULTICAST_GROUP=64

class multicast_next_hop_entry(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint16),
                ('next_hop',c_int16),
                ('label_to_push',c_uint32)]
    def __str__(self):
        ret=dict()
        ret['is_valid']=self.is_valid
        ret['next_hop']=self.next_hop
        ret['label_to_push']=self.label_to_push
        return str(ret)
    
class multicast_next_hops(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint16),
                ('nr_hops',c_int16),
                ('index',c_int16),
                ('ref_cnt',c_int16),
                ('nexthops',multicast_next_hop_entry*MAX_HOPS_IN_MULTICAST_GROUP)]
    def __str__(self):
        ret=dict()
        ret['is_valid']=self.is_valid
        ret['nr_hops']=self.nr_hops
        ret['index']=self.index
        ret['ref_cnt']=self.ref_cnt
        lst=list()
        for i in range(MAX_HOPS_IN_MULTICAST_GROUP):
            if self.nexthops[i].is_valid==0:
                continue
            lst.append(str(self.nexthops[i]))
        ret['nexthops']=lst
        return str(ret)

    def dump_definition(self):
        print('size of multicast_next_hops:',sizeof(multicast_next_hops))
        print(multicast_next_hops.is_valid,'is_valid')
        print(multicast_next_hops.nr_hops,'nr_hops')
        print(multicast_next_hops.index,'index')
        print(multicast_next_hops.ref_cnt,'ref_cnt')
        print(multicast_next_hops.nexthops,'nexthops')
    
 
def register_mnexthop():
    api_ret=c_int64(0)
    rc=clib.spine_api_register_mnexthop(byref(api_ret))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value<0:
        raise api_return_exception('failed to register mnexthops entry with api_ret:%x'%(api_ret.value))
    return api_ret.value
def get_mnexthop(index):
    api_ret=c_int64(0)
    _index=c_int16(index)
    m=multicast_next_hops()
    rc=clib.spine_api_get_mnexthop(byref(api_ret),_index,byref(m))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('api_ret:%x'%(api_ret.value))
    return m

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    #multicast_next_hops().dump_definition()
    print(register_mnexthop())
    print(register_mnexthop())
    print(get_mnexthop(0))
    print(get_mnexthop(1))
    print(get_mnexthop(2047))
