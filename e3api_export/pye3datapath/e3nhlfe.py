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
    index=0
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
        ret['index']=self.index
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
        n.index=self.index
        return n

class Nexthop(Structure):
    _pack_=1
    _fields_=[('local_e3iface_index',c_uint32),
                ('remote_neighbor_index',c_uint32),
                ('is_valid',c_uint8),
                ('reserved0',c_uint8)]
    index=0
    def __str__(self):
        ret=dict()
        ret['local_e3iface_index']=self.local_e3iface_index
        ret['remote_neighbor_index']=self.remote_neighbor_index
        ret['is_valid']=self.is_valid
        ret['index']=self.index
        return repr(ret)
    def dump_definition(self):
        print(Nexthop.local_e3iface_index)
        print(Nexthop.remote_neighbor_index)
        print(Nexthop.is_valid)
        print(Nexthop.reserved0)
    def clone(self):
        h=Nexthop()
        h.local_e3iface_index=self.local_e3iface_index
        h.remote_neighbor_index=self.remote_neighbor_index
        h.is_valid=self.is_valid
        h.reserved0=self.reserved0
        h.index=self.index
        return h

class MNexthops(Structure):
    _pack_=1
    _fields_=[('multicast_group_id',c_uint64),
                ('is_valid',c_uint8),
                ('nr_hops',c_uint8),
                ('next_hops',c_uint16*64)]
    index=0
    def __str__(self):
        ret=dict()
        ret['multicast_group_id']=self.multicast_group_id
        ret['is_valid']=self.is_valid
        ret['nr_hops']=self.nr_hops
        lst=list()
        for i in range(self.nr_hops):
            lst.append(self.next_hops[i])
        ret['next_hops']=str(lst)
        ret['index']=self.index
        return repr(ret)
    def dump_definition(self):
        print(MNexthops.multicast_group_id)
        print(MNexthops.is_valid)
        print(MNexthops.nr_hops)
        print(MNexthops.next_hops)
    def clone(self):
        mnh=MNexthops()
        mnh.multicast_group_id=self.multicast_group_id
        mnh.is_valid=self.is_valid
        mnh.nr_hops=self.nr_hops
        for i in range(64):
            mnh.next_hops[i]=self.next_hops[i]
        mnh.index=self.index
        return mnh



def get_neighbour(index):
    api_ret=c_uint64(0)
    index_to_pass=c_uint16(index)
    nbr=Neighbour()
    rc=clib.get_neighbour(byref(api_ret),index_to_pass,byref(nbr))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    nbr.index=index
    return nbr
   
def register_neighbour(ip_string,mac_string):
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

def refresh_neighbour_mac(ip_string,mac_string):
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
    class B(Structure):
        _pack_=1
        _fields_=[('b',c_uint16*nr_entries_per_fetch)]
    a=A()
    b=B()
    lst=list()
    while True:
        rc=clib.list_neighbours_partial(byref(api_ret),byref(index_to_start),byref(nr_entries),byref(a),byref(b))
        if rc!=0:
            raise api_call_exception()
        for i in range(nr_entries.value):
            nbr=a.a[i].clone()
            nbr.index=b.b[i]
            lst.append(nbr)
        if nr_entries.value !=nr_entries_per_fetch:
            break
    return lst

def delete_neighbour(ip_string):
    api_ret=c_uint64(0);
    _ip_string=create_string_buffer(16)
    _ip_string.value=ip_string.encode()
    rc=clib.delete_neighbour(byref(api_ret),_ip_string)
    if rc!=0:
        raise api_call_exception()

def register_nexthop(iface,ip_string):
    api_ret=c_uint64(0)
    _iface=c_uint16(iface)
    _ip_string=create_string_buffer(16)
    _ip_string.value=ip_string.encode()
    index=c_uint16(0)
    rc=clib.register_nexthop(byref(api_ret),_iface,_ip_string,byref(index)) 
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    return index.value
def get_nexthop(index):
    api_ret=c_uint64(0)
    _index=c_uint16(index)
    nh=Nexthop()
    rc=clib.get_nexthop(byref(api_ret),_index,byref(nh))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    nh.index=index
    return nh
def list_nexthops():
    nr_entries_per_fetch=64
    api_ret=c_uint64(0)
    index_to_start=c_uint32(0)
    nr_entries=c_uint32(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',Nexthop*nr_entries_per_fetch)]
    class B(Structure):
        _pack_=1
        _fields_=[('b',c_uint16*nr_entries_per_fetch)]
    a=A()
    b=B()
    lst=list()
    while True:
        rc=clib.list_nexthop_partial(byref(api_ret),byref(index_to_start),byref(nr_entries),byref(a),byref(b))
        if rc!=0:
            raise api_call_exception()
        for i in range(nr_entries.value):
            nh=a.a[i].clone()
            nh.index=b.b[i]
            lst.append(nh)
        if nr_entries.value !=nr_entries_per_fetch:
            break
    return lst        

def delete_nexthop(index):
    api_ret=c_uint64(0)
    _index=c_uint16(index)
    rc=clib.delete_nexthop(byref(api_ret),_index)
    if rc!=0:
        raise api_call_exception()
 
def register_multicast_nexthops(hops_lst):
    api_ret=c_uint64(0)
    mnh=MNexthops()
    index=c_uint16(0)
    if len(hops_lst) > 64:
        raise api_return_exception()
    for i in range(len(hops_lst)):
        mnh.next_hops[i]=hops_lst[i]
    mnh.nr_hops=len(hops_lst)
    rc=clib.register_mnexthops(byref(api_ret),byref(mnh),byref(index))
    if rc !=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    return index.value

def get_multicast_nexthops(index):
    api_ret=c_uint64(0)
    mnh=MNexthops()
    _index=c_uint16(index)
    rc=clib.get_mnexthops(byref(api_ret),_index,byref(mnh))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    mnh.index=index
    return mnh

def list_multicast_nexthops():
    nr_entries_per_fetch=64
    api_ret=c_uint64(0)
    index_to_start=c_uint32(0)
    nr_entries=c_uint32(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',MNexthops*nr_entries_per_fetch)]
    class B(Structure):
        _pack_=1
        _fields_=[('b',c_uint16*nr_entries_per_fetch)]
    a=A()
    b=B()
    lst=list()
    while True:
        rc=clib.list_mnexthops_partial(byref(api_ret),byref(index_to_start),byref(nr_entries),byref(a),byref(b))
        if rc!=0:
            raise api_call_exception()
        for i in range(nr_entries.value):
            nh=a.a[i].clone()
            nh.index=b.b[i]
            lst.append(nh)
        if nr_entries.value !=nr_entries_per_fetch:
            break
    return lst
def delete_multicast_nexthops(index):
    api_ret=c_uint64(0)
    _index=c_uint16(index)
    rc=clib.delete_mnexthops(byref(api_ret),_index)
    if rc!=0:
        raise api_call_exception()

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    delete_multicast_nexthops(0)
    delete_multicast_nexthops(1)
    print(register_multicast_nexthops([1,2,3]))
    print(register_multicast_nexthops([1,3,2,6]))
   
    #print(get_multicast_nexthops(0))
    #print(get_multicast_nexthops(1))
    #print(get_multicast_nexthops(1023))
    #print(get_multicast_nexthops(1024))
    #MNexthops().dump_definition()
    #print(Neighbour().dump_definition())
    #delete_neighbour('13.140.150.1')
    #delete_neighbour('0.18.47.54')
    print(register_neighbour('130.140.150.1','0:01:23:24:35:45'))
    print(register_neighbour('130.140.150.2','01:01:23:24:35:45'))
    #print(get_neighbour(0))
    #print(get_neighbour(63))
    #print(get_nexthop(1024))
    #print(get_neighbour(1023))
    #print(get_neighbour(1024))
    #print(refresh_neighbor_mac('130.140.150.1','0:01:23:24:35:46'))
    #delete_neighbour('13.140.150.1')
    #print(register_nexthop(1,'130.140.150.1'))
    #print(register_nexthop(2,'130.140.150.1'))
    #print(get_nexthop(0))
    #print(register_nexthop(2,'130.140.150.2'))
    #print(Nexthop().clone())
    #delete_nexthop(0)
    #delete_nexthop(1)
    #delete_nexthop(2)
    for n in list_neighbours():
        print(n)
    for nh in list_nexthops():
        print(nh)
    for mnh in list_multicast_nexthops():
        print(mnh)
