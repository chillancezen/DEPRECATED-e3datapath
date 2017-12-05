#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint
MAX_E_LAN_SERVICES=4096
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
    def tabulate(self):
        lst=list()
        lst.append(['is_valid',self.is_valid])
        lst.append(['iface',self.iface])
        lst.append(['vlan_tci',self.vlan_tci])
        lst.append(['reserved0',self.reserved0])
        return lst
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
            ports_lst.append(str(self.ports[i]))
        ret['ports']=ports_lst
        nhlfe_lst=list()
        for i in range(MAX_NHLFE_IN_E_LAN_SERVICE):
            if self.nhlfes[i].is_valid==0:
                continue
            nhlfe_lst.append(str(self.nhlfes[i]))
        ret['nhlfes']=nhlfe_lst
        return str(ret)
    def tabulate(self):
        table=list()
        table.append(['is_valid',self.is_valid])
        table.append(['is_releasing',self.is_releasing])
        table.append(['index',self.index])
        table.append(['ref_cnt',self.ref_cnt])
        table.append(['nr_ports',self.nr_ports])
        table.append(['nr_nhlfes',self.nr_nhlfes])
        table.append(['multicast_NHLFE',self.multicast_NHLFE])
        table.append(['multicast_label',self.multicast_label])
        table.append(['fib_base','0x%x'%(self.fib_base)])
        ports_str=''
        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            if self.ports[i].is_valid==0:
                continue
            ports_str=ports_str+str(self.ports[i])+'\n'
        table.append(['ports',ports_str])
        nhlfes_str=''
        for i in range(MAX_NHLFE_IN_E_LAN_SERVICE):
            if self.nhlfes[i].is_valid==0:
                continue
            nhlfes_str=nhlfes_str+str(self.nhlfes[i])+'\n'
        table.append(['nhlfes',nhlfes_str])
        print(tabulate.tabulate(table,['Field','Value'],tablefmt='psql'))
    def dump_definition(self):
        print('size of ether_lan:',sizeof(ether_lan))
        print(ether_lan.is_valid,'is_valid')
        print(ether_lan.is_releasing,'is_releasing')
        print(ether_lan.index,'index')
        print(ether_lan.ref_cnt,'ref_cnt')
        print(ether_lan.nr_ports,'nr_ports')
        print(ether_lan.nr_nhlfes,'nr_nhlfes')
        print(ether_lan.multicast_NHLFE,'multicast_NHLFE')
        print(ether_lan.multicast_label,'multicast_label')
        print(ether_lan.fib_base,'fib_base')
        print(ether_lan.ports,'ports')
        print(ether_lan.nhlfes,'nhlfes')
        print(ether_lan.reserved0,'reserved0')

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
def list_ether_lan_services():
    lst=list()
    api_ret=c_int64(0)
    nr_entries=c_int32(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',c_int16*MAX_E_LAN_SERVICES)]
    a=A()
    rc=clib.leaf_api_list_e_services(byref(api_ret),0,byref(nr_entries),byref(a))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('bug on this')
    for i in range(nr_entries.value):
        lst.append(i)
    return lst

'''
return the actual inner port id if successful,
otherwise a api_return_exception() exception is raised
,register_ether_lan_nhlfe() has the same behavior
'''
def register_ether_lan_port(elan_index,e3iface,vlan_tci):
    api_ret=c_int64(0)
    _elan_index=c_int16(elan_index)
    _e3iface=c_int32(e3iface)
    _vlan_tci=c_int32(vlan_tci)
    rc=clib.leaf_api_register_e_service_port(byref(api_ret),0,_elan_index,_e3iface,_vlan_tci)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value<0:
        raise api_return_exception('failed to register an port for e-lan service with api_ret:%x'%(api_ret.value))
    return api_ret.value
def register_ether_lan_nhlfe(elan_index,nhlfe,label_to_push):
    api_ret=c_int64(0)
    _elan_index=c_int16(elan_index)
    _nhlfe=c_int32(nhlfe)
    _label_to_push=c_int32(label_to_push)
    rc=clib.leaf_api_register_e_service_nhlfe(byref(api_ret),0,_elan_index,_nhlfe,_label_to_push)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value<0:
        raise api_return_exception('failed to register an nhlfe for e-lan service with api_ret:%x'%(api_ret.value))
    return api_ret.value
def delete_ether_lan_port(elan_index,inner_port_id):
    api_ret=c_int64(0)
    _elan_index=c_int16(elan_index)
    _inner_port_id=c_int16(inner_port_id)
    rc=clib.leaf_api_delete_e_service_port(byref(api_ret),0,_elan_index,_inner_port_id)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to delete an inner port of e-lan with api_ret:%x'%(api_ret.value))
def delete_ether_lan_nhlfe(elan_index,inner_nhlfe_id):
    api_ret=c_int64(0)
    _elan_index=c_int16(elan_index)
    _inner_nhlfe_id=c_int16(inner_nhlfe_id)
    rc=clib.leaf_api_delete_e_service_nhlfe(byref(api_ret),0,_elan_index,_inner_nhlfe_id)
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('failed to delete an inner nhlfe of e-lan with api_ret:%x'%(api_ret.value))

if __name__=='__main__':
    ether_lan().dump_definition()
    from pye3datapath.common.neighbor import *
    from pye3datapath.common.nexthop import *
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    register_neighbor('130.140.150.1','08:00:27:ab:24:62')
    register_nexthop(0,0)
    #print(port_entry().dump_definition())
    #print(nhlfe_entry().dump_definition())
    #print(sizeof(ether_lan))
    #print(ether_lan())
    print(register_ether_lan_service())
    print(register_ether_lan_port(0,0,123))
    print(register_ether_lan_port(0,1,0))
    print(register_ether_lan_port(0,1,1212))
    print(register_ether_lan_port(0,112,0))
    print(register_ether_lan_nhlfe(0,0,1024))
    #delete_ether_lan_port(0,0)
    #delete_ether_lan_nhlfe(0,0)   
    #print(get_ether_lan_service(0))
    #print(get_ether_lan_service(1))
    print(register_ether_lan_service())
    print(register_ether_lan_service())
    for elan in list_ether_lan_services():
        get_ether_lan_service(elan).tabulate()
