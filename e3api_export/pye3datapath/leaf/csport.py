#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint


class csport_dist_entry(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint8),
                ('e_service',c_int8),
                ('service_index',c_int16)]
    vlan=0
    def __str__(self):
        ret=dict()
        ret['is_valid']=self.is_valid
        ret['e_service']=self.e_service
        ret['service_index']=self.service_index
        ret['vlan']=self.vlan
        return str(ret)
    def clone(self):
        c=csport_dist_entry()
        c.is_valid=self.is_valid
        c.e_service=self.e_service
        c.service_index=self.service_index
        c.vlan=self.vlan
        return c
    def dump_definition(self):
        print('size of csport_dist_entry:',sizeof(csport_dist_entry))
        print(csport_dist_entry.is_valid,'is_valid')
        print(csport_dist_entry.e_service,'e_service')
        print(csport_dist_entry.service_index,'service_index')
    

def attach_csport_to_eline(iface,vlan,eline):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _vlan=c_int16(vlan)
    _eline=c_int16(eline)
    rc=clib.leaf_api_csp_setup_port(byref(api_ret),_iface,_vlan,1,_eline)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to set up the port with api_ret:%x'%(api_ret.value))
def attach_csport_to_elan(iface,vlan,elan):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _vlan=c_int16(vlan)
    _elan=c_int16(elan)
    rc=clib.leaf_api_csp_setup_port(byref(api_ret),_iface,_vlan,0,_elan)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to set up the port with api_ret:%x'%(api_ret.value))
def detach_csport(iface,vlan):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _vlan=c_int16(vlan)
    rc=clib.leaf_api_csp_withdraw_port(byref(api_ret),_iface,_vlan)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('can not withdraw port vlan entry with api_ret:%x'(api_ret.value))
CSP_TABLE_NR_ENTRIES_PER_FETCH=2048
CSP_TABLE_NR_BLOCK=2
def list_cspirt_dist_table(iface):
    api_ret=c_int64(0)
    _iface=c_int16(iface)
    _block_index=c_int16(0)
    lst=list()
    class A(Structure):
        _pcak_=1
        _fields_=[('a',csport_dist_entry*CSP_TABLE_NR_ENTRIES_PER_FETCH)]
    a=A()
    for i in range(CSP_TABLE_NR_BLOCK):
        _block_index=c_int16(i)
        rc=clib.leaf_api_list_csp_distribution_table(byref(api_ret),_iface,_block_index,byref(a))
        if rc!=0:
            raise api_call_exception()
        if api_ret.value!=0:
            raise api_return_exception('can not fetch csport\'s distribution table with api_ret:%x'%(api_ret.value))
        for entry_index in range(CSP_TABLE_NR_ENTRIES_PER_FETCH):
            if a.a[entry_index].is_valid==0:
                continue
            c=a.a[entry_index].clone()
            c.vlan=entry_index+i*CSP_TABLE_NR_ENTRIES_PER_FETCH
            lst.append(c)
    return lst

if __name__=='__main__':
    from pye3datapath.e3iface import *
    from pye3datapath.common.neighbor import *
    from pye3datapath.common.nexthop import *
    from pye3datapath.leaf.etherline import *
    from pye3datapath.leaf.etherlan import *
    csport_dist_entry().dump_definition()
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    register_neighbor('130.140.150.1','08:00:27:ab:24:62')
    register_nexthop(0,0)

    print(register_ether_line_service())
    print(register_ether_lan_service())
    print(attach_e3iface('0000:02:05.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,True))
     
    attach_csport_to_eline(0,100,0)
    attach_csport_to_elan(0,100,0)
    attach_csport_to_eline(0,100,0)
    attach_csport_to_eline(0,3249,0)
    #detach_csport(0,100)
    #delete_ether_lan_service(0)
    #delete_ether_line_service(0)
    print(get_ether_line_service(0))
    get_ether_lan_service(0).tabulate()
    for entry in list_cspirt_dist_table(0):
        print(entry)

    #for iface in get_e3iface_list():
        #get_e3iface(iface).tabulate()
