#! /usr/bin/python3
from ctypes import *
import tabulate
from pye3datapath.e3util import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

class nexthop(Structure):
    _pack_=1
    _fields_=[('local_e3iface',c_int16),
                ('common_neighbor_index',c_int16),
                ('ref_cnt',c_int16),
                ('index',c_int16),
                ('is_valid',c_int8),
                ('reserved0',c_uint8),
                ('reserved1',c_uint16)]
    def __init__(self):
        pass
    def __str__(self):
        ret=dict()
        ret['local_e3iface']=self.local_e3iface
        ret['common_neighbor_index']=self.common_neighbor_index
        ret['ref_cnt']=self.ref_cnt
        ret['index']=self.index
        ret['is_valid']=self.is_valid
        ret['reserved0']=self.reserved0
        ret['reserved1']=self.reserved1
        return str(ret)
    def dump_definition(self):
        print('size of nexthop:',sizeof(nexthop))
        print(nexthop.local_e3iface,'local_e3iface')
        print(nexthop.common_neighbor_index,'common_neighbor_index')
        print(nexthop.ref_cnt,'ref_cnt')
        print(nexthop.index,'index')
        print(nexthop.is_valid,'is_valid')
        print(nexthop.reserved0,'reserved0')
        print(nexthop.reserved1,'reserved1')
    def clone(self):
        n=nexthop()
        n.local_e3iface=self.local_e3iface
        n.common_neighbor_index=self.common_neighbor_index
        n.ref_cnt=self.ref_cnt
        n.index=self.index
        n.is_valid=self.is_valid
        n.reserved0=self.reserved0
        n.reserved1=self.reserved1
        return n


def register_nexthop(e3iface,neighbor_index):
    api_ret=c_int64(0)
    _e3iface=c_int16(e3iface)
    _neighbor_index=c_int16(neighbor_index)
    rc=clib.e3net_api_register_common_nexthop(byref(api_ret),_e3iface,_neighbor_index)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value<0:
        raise api_return_exception('fails to register common nexthop with api_ret:%x'%(api_ret.value))
    return api_ret.value
def get_nexthop(nexthop_index):
    api_ret=c_int64(0)
    _nexthop_index=c_int16(nexthop_index)
    n=nexthop()
    rc=clib.e3net_api_get_common_nexthop(byref(api_ret),_nexthop_index,byref(n))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('can not retrieve a nexthop entry with api_ret:%x'%(api_ret.value))
    return n

MAX_NR_NEXTHOPS_PER_FETCH=192

def list_nexthops():
    api_ret=c_int64(0)
    index_to_start=c_int16(0)
    nr_entries=c_int16(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',nexthop*MAX_NR_NEXTHOPS_PER_FETCH)]
    a=A()
    lst=list()
    while True:
        rc=clib.e3net_api_list_common_nexthop_partial(byref(api_ret),byref(index_to_start),byref(nr_entries),byref(a))
        if rc!=0:
            raise api_call_exception()
        for i in range(nr_entries.value):
            lst.append(a.a[i].clone())
        if nr_entries.value!=MAX_NR_NEXTHOPS_PER_FETCH:
            break
    return lst
def delete_nexthop(nexthop_index):
    api_ret=c_int64(0) 
    _nexthop_index=c_int16(nexthop_index)
    rc=clib.e3net_api_delete_common_nexthop(byref(api_ret),_nexthop_index)
    if rc !=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('deleting next hop fails with api_ret:%x'%(api_ret.value)) 
def tabulate_nexthops():
    table=list()
    for n in list_nexthops():
        table.append([n.index,
            n.ref_cnt,
            n.local_e3iface,
            n.common_neighbor_index])
    print(tabulate.tabulate(table,['index','ref_cnt','e3iface','neighbor index'],tablefmt='psql'))

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    from pye3datapath.common.neighbor import *
    print(nexthop().dump_definition())
    print(register_neighbor('130.140.250.1','02:42:7e:5f:17:ee'))
    print(register_nexthop(0,0))
    print(register_nexthop(12,0))
    #delete_nexthop(0)
    #delete_neighbor(0)
    for n in list_nexthops():
        print(n)
    for m in list_neighbors():
        print(m)
    tabulate_nexthops()
