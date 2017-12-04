#! /usr/bin/python3
from ctypes import *
from pye3datapath.e3util import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

class neighbor(Structure):
    _pack_=1
    _fields_=[('neighbour_ip_as_le',c_uint8*4),
                ('ref_cnt',c_int16),
                ('index',c_int16),
                ('mac',c_uint8*6),
                ('is_valid',c_uint8),
                ('reserved0',c_uint8)]
    def __init__(self):
        pass
    def _neighbor_ip_to_string(self):
        return '%d.%d.%d.%d'%(self.neighbour_ip_as_le[3],
                    self.neighbour_ip_as_le[2],
                    self.neighbour_ip_as_le[1],
                    self.neighbour_ip_as_le[0])
    def _mac_to_string(self):
        return '%02x:%02x:%02x:%02x:%02x:%02x'%(self.mac[0],
                self.mac[1],
                self.mac[2],
                self.mac[3],
                self.mac[4],
                self.mac[5])
    def __str__(self):
        ret=dict()
        ret['neighbour_ip_as_le']=self._neighbor_ip_to_string()
        ret['ref_cnt']=self.ref_cnt
        ret['index']=self.index
        ret['mac']=self._mac_to_string()
        ret['is_valid']=self.is_valid
        ret['reserved0']=self.reserved0
        return str(ret)
    def dump_definition(self):
        print('size of neighbor:',sizeof(neighbor))
        print(neighbor.neighbour_ip_as_le,'neighbor.neighbour_ip_as_le')
        print(neighbor.ref_cnt,'neighbor.ref_cnt')
        print(neighbor.index,'neighbor.index')
        print(neighbor.mac,'neighbor.mac')
        print(neighbor.is_valid,'neighbor.is_valid')
        print(neighbor.reserved0,'neighbor.reserved0')
    def clone(self):
        n=neighbor()
        n.neighbour_ip_as_le=self.neighbour_ip_as_le
        n.ref_cnt=self.ref_cnt
        n.index=self.index
        n.mac=self.mac
        n.is_valid=self.is_valid
        n.reserved0=self.reserved0
        return n
 
'''
register a topological neighbor, if successful
return the actual index which is equal or greater than 0,
otherwise a negative value is returned
'''
def register_neighbor(ip,mac):
    api_ret=c_int64(0)
    ip_string=create_string_buffer(E3API_IP_STRING_LENGTH)
    mac_string=create_string_buffer(E3API_MAC_STRING_LENGTH)
    ip_string.value=ip.encode()
    mac_string.value=mac.encode()
    rc=clib.e3net_api_register_or_update_common_neighbor(byref(api_ret),1,ip_string,mac_string)
    if rc!=0:
        raise api_call_exception(rc)
    if api_ret.value<0:
        raise api_return_exception('registering common neighbor fails with api_ret:%x'%(api_ret.value))
    return api_ret.value
def update_neighbor_mac(ip,mac):
    api_ret=c_int64(0)
    ip_string=create_string_buffer(E3API_IP_STRING_LENGTH)
    mac_string=create_string_buffer(E3API_MAC_STRING_LENGTH)
    ip_string.value=ip.encode()
    mac_string.value=mac.encode()
    rc=clib.e3net_api_register_or_update_common_neighbor(byref(api_ret),0,ip_string,mac_string)
    if rc!=0:
        raise api_call_exception(rc)
    if api_ret.value!=0:
        raise api_return_exception('updating neighbor fails with api_ret:%x'%(api_ret.value))
def get_neighbor(neighbor_index):
    api_ret=c_int64(0)
    _neighbor_index=c_int16(neighbor_index)
    n=neighbor()
    rc=clib.e3net_api_get_common_neighbor(byref(api_ret),_neighbor_index,byref(n))
    if rc!=0:
        raise api_call_exception(rc)
    if api_ret.value<0:
        raise api_return_exception('failing to retrieve a common neighbor with api_ret:%d'%(api_ret.value))
    return n

MAX_NR_NEIGHBORS_PER_FETCH=128

def list_neighbors():
    api_ret=c_int64(0)
    index_to_start=c_int16(0)
    nr_entries=c_int16(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',neighbor*MAX_NR_NEIGHBORS_PER_FETCH)]
    a=A()
    lst=list()
    while True:
        rc=clib.e3net_api_list_common_neighbor_partial(byref(api_ret),byref(index_to_start),byref(nr_entries),byref(a))
        if rc!=0:
            raise api_call_exception(rc)
        for i in range(nr_entries.value):
            lst.append(a.a[i].clone())
        if nr_entries.value!=MAX_NR_NEIGHBORS_PER_FETCH:
            break
    return lst
def delete_neighbor(neighbor_index):
    api_ret=c_int64(0)
    _neighbor_index=c_int16(neighbor_index)
    rc=clib.e3net_api_delete_common_neighbor(byref(api_ret),_neighbor_index)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('neighbor deletion fails due to api_ret:%d'%(api_ret.value))

     
if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    #neighbor().dump_definition()
    #print(register_neighbor('2.3.255.232','09:22:2:3:43:2'))
    #print(register_neighbor('2.3.255.234','09:22:2:3:43:2'))
    #print(get_neighbor(0))
    #print(get_neighbor(1))
    for i in range(5):
        register_neighbor('2.3.255.%d'%(i),'09:22:2:3:43:2')
    update_neighbor_mac('2.3.255.0','00:0c:29:34:77:b7')
    delete_neighbor(0)
    #update_neighbor_mac('2.3.255.5','00:0c:29:34:77:b7')
    for n in list_neighbors():
        print(n)
    #n=neighbor()
    #n.neighbour_ip_as_le[0]=0xff
    #n.neighbour_ip_as_le[1]=0xff
    #n.neighbour_ip_as_le[2]=0xff
    #m=n.clone()
    #n.neighbour_ip_as_le[3]=0x1
    #print(n,m)
