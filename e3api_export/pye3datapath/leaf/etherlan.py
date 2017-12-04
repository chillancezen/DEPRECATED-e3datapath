#! /usr/bin/python3
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

MAX_PORTS_IN_E_LAN_SERVICE=64
MAX_NHLFE_IN_E_LAN_SERVICE=64
class port_entry(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint16),
                ('iface',c_int16),
                ('vlan_tci',c_uint16),
                ('reserved0',c_uint16)]
    def __str__(self):
        ret=dict()
        ret['is_valid']=self.is_valid
        ret['iface']=self.iface
        ret['vlan_tci']=self.vlan_tci
        ret['reserved0']=self.reserved0
        return str(ret)
    def dump_definition(self):
        print('size of port entry:',sizeof(port_entry))
        print(port_entry.is_valid,'is_valid')
        print(port_entry.iface,'iface')
        print(port_entry.vlan_tci,'vlan_tci')
        print(port_entry.reserved0,'reserved0')
class nhlfe_entry(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint16),
                ('NHLFE',c_int16),
                ('label_to_push',c_uint32)]
    def __str__(self):
        ret=dict()
        ret['is_valid']=self.is_valid
        ret['NHLFE']=self.NHLFE
        ret['label_to_push']=self.label_to_push
        return str(ret)
    def dump_definition(self):
        print('size of nhlfe entry:',sizeof(nhlfe_entry))
        print(nhlfe_entry.is_valid,'is_valid')
        print(nhlfe_entry.NHLFE,'NHLFE')
        print(nhlfe_entry.label_to_push,'label_to_push')

class ether_lan(Structure):
    _pack_=1
    _fields_=[('is_valid',c_uint8),
                ('is_releasing',c_uint8),
                ('index',c_int16),
                ('ref_cnt',c_int16),
                ('nr_ports',c_int16),
                ('nr_nhlfes',c_int16),
                ('multicast_NHLFE',c_int16),
                ('multicast_label',c_uint32),
                ('fib_base',c_uint64),
                ('ports',port_entry*MAX_PORTS_IN_E_LAN_SERVICE),
                ('nhlfes',nhlfe_entry*MAX_NHLFE_IN_E_LAN_SERVICE),
                ('reserved0',c_uint8*20)]
    def __str__(self):
        ret=dict()
        ret['is_valid']=self.is_valid
        ret['is_releasing']=self.is_releasing
        ret['index']=self.index
        ret['ref_cnt']=self.ref_cnt
        ret['nr_ports']=self.nr_ports
        ret['nr_nhlfes']=self.nr_nhlfes
        ret['multicast_NHLFE']=self.multicast_NHLFE
        ret['multicast_label']=self.multicast_label
        ret['fib_base']='0x%x'%(self.fib_base)
        ports_lst=list()
        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            if self.ports[i].is_valid==0:
                continue
            ports_lst.append(self.ports[i])
        ret['ports']=ports_lst
        nhlfe_lst=list()
        for i in range(MAX_NHLFE_IN_E_LAN_SERVICE):
            if self.nhlfes[i].is_valid==0:
                continue
            nhlfe_lst.append(self.nhlfes[i])
        ret['nhlfes']=nhlfe_lst
        return str(ret)

def register_ether_lan_service():
    api_ret=c_int64(0)
    elan_number=c_int32(0)
    rc=clib.leaf_api_register_e_service(byref(api_ret),0,byref(elan_number))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to register an e-lan service with api_ret:%x'%(api_ret.value))
    return elan_number.value
def get_ether_lan_service(elan_index):
    api_ret=c_int64(0)
    _elan_index=c_int16(elan_index)
    elan=ether_lan()
    rc=clib.leaf_api_get_e_lan(byref(api_ret),_elan_index,byref(elan))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to retrieve e-lan service with api_ret:%x'%(api_ret.value))
    return elan

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    #print(port_entry().dump_definition())
    #print(nhlfe_entry().dump_definition())
    #print(sizeof(ether_lan))
    #print(ether_lan())
    print(register_ether_lan_service())
    print(get_ether_lan_service(0))
    print(get_ether_lan_service(1))
