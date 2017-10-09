from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint
class Neighbour(Structure):
    _pack_=1
    _fields_=[('neighbor_ip',c_uint8*4),
             ('mac',c_uint8*6),
             ('is_valid',c_uint8),
             ('reserved0',c_uint8)]
    def neighbor_ip_to_string(self):
        return '%d.%d.%d.%d'%(self.neighbor_ip[3],
                self.neighbor_ip[2],
                self.neighbor_ip[1],
                self.neighbor_ip[0])
    def mac_to_string(self):
        return '%02x:%02x:%02x:%02x:%02x:%02x'%(self.mac[0],
                self.mac[1],
                self.mac[2],
                self.mac[3],
                self.mac[4],
                self.mac[5])
    def __str__(self):
        ret=dict()
        ret['neighbor_ip']=self.neighbor_ip_to_string()
        ret['mac']=self.mac_to_string()
        ret['is_valid']=self.is_valid
        return repr(ret)
    def dump_definition(self):
        print(Neighbour.neighbor_ip)
        print(Neighbour.mac)
        print(Neighbour.is_valid)
        print(Neighbour.reserved0)
    def clone(self):
        n=Neighbour()
        n.neighbor_ip=self.neighbor_ip
        n.mac=self.mac
        n.is_valid=self.is_valid
        n.reserved0=self.reserved0
        return n

    
def register_neighbor(ip_string,mac_string):
    '''
    ip_string as c_uint8*16
    mac_string as c_uint8*18
    '''
    api_ret=c_uint64(0);
    _ip_string=create_string_buffer(16)
    _mac_string=create_string_buffer(18)
    _ip_string.value=ip_string.encode()
    _mac_string.value=mac_string.encode()
    index=c_uint16(0)
    rc=clib.register_neighbor(byref(api_ret),_ip_string,_mac_string,byref(index)) 
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    return index.value
def refresh_neighbor_mac(ip_string,mac_string):
    api_ret=c_uint64(0);
    _ip_string=create_string_buffer(16)
    _mac_string=create_string_buffer(18)
    _ip_string.value=ip_string.encode()
    _mac_string.value=mac_string.encode()
    rc=clib.refresh_neighbor_mac(byref(api_ret),_ip_string,_mac_string)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
def list_neighbours():
    nr_entries_per_fetch=64
    api_ret=c_uint64(0);#if calling succeed, 0 always is returned
    index_to_start=c_uint32(0)
    nr_entries=c_uint32(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',Neighbour*nr_entries_per_fetch)]
    a=A()
    lst=list()
    while True:
        rc=clib.list_neighbours_partial(byref(api_ret),byref(index_to_start),byref(nr_entries),byref(a))
        if rc!=0:
            raise api_call_exception()
        for i in range(nr_entries.value):
            lst.append(a.a[i].clone())
        if nr_entries.value !=nr_entries_per_fetch:
            break
    return lst
if __name__=='__main__':
    register_service_endpoint('tcp://localhost:507')
    #print(Neighbour().dump_definition())
    print(register_neighbor('13.140.150.1','0:01:23:24:35:45'))
    print(register_neighbor('130.140.150.1','01:01:23:24:35:45'))
    #print(refresh_neighbor_mac('130.140.150.1','0:01:23:24:35:46'))
    for n in list_neighbours():
        print(n)
