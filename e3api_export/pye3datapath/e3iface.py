#! /usr/bin/python3
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint


E3IFACE_MODEL_GENERIC_SINGLY_QUEUE=0
E3IFACE_ROLE_PROVIDER_BACKBONE_PORT=0
E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT=1
E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT=2


class E3Interface(Structure):
    _pack_=1
    _fields_=[('name',c_char*64),
                ('hwiface_model',c_uint8),
                ('hwiface_role',c_uint8),
                ('reserved0',c_uint8),
                ('iface_status',c_uint8),
                ('nr_queues',c_uint8),
                ('under_releasing',c_uint8),
                ('has_peer_device',c_uint8),
                ('lsc_enabled',c_uint8),
                ('port_id',c_uint16),
                ('peer_port_id',c_uint16),
                ('mac_addrs',c_uint8*6),
                ('input_node',c_uint16*8),
                ('output_node',c_uint16*8),
                ('_dummy0',c_uint8*78)]
    def __str__(self):
        ret=dict()
        ret['name']=self.name
        ret['hwiface_model']=self.hwiface_model
        ret['hwiface_role']=self.hwiface_role
        ret['reserved0']=self.reserved0
        ret['iface_status']=self.iface_status
        ret['nr_queues']=self.nr_queues
        ret['under_releasing']=self.under_releasing
        ret['has_peer_device']=self.has_peer_device
        ret['lsc_enabled']=self.lsc_enabled
        ret['port_id']=self.port_id
        ret['peer_port_id']=self.peer_port_id
        ret['mac_addrs']='%02x:%02x:%02x:%02x:%02x:%02x'%(self.mac_addrs[0],
                    self.mac_addrs[1],
                    self.mac_addrs[2],
                    self.mac_addrs[3],
                    self.mac_addrs[4],
                    self.mac_addrs[5])
        input_lst=list()
        output_lst=list()
        for i in range(self.nr_queues):
            input_lst.append(self.input_node[i])
            output_lst.append(self.output_node[i])
        ret['input_node']=input_lst
        ret['output_node']=output_lst
        #for item  in self._fields_:
            #ret[item[0]]=getattr(self,item[0])
        #ret['raw']=repr(string_at(addressof(self),sizeof(self)))
        return repr(ret)
    def dump_definition(self):
        print('%20s%s'%('name:',E3Interface.name))
        print('%20s%s'%('hwiface_model:',E3Interface.hwiface_model))
        print('%20s%s'%('hwiface_role:',E3Interface.hwiface_role))
        print('%20s%s'%('reserved0:',E3Interface.reserved0))
        print('%20s%s'%('iface_status:',E3Interface.iface_status))
        print('%20s%s'%('nr_queues:',E3Interface.nr_queues))
        print('%20s%s'%('under_releasing:',E3Interface.under_releasing))
        print('%20s%s'%('has_peer_device:',E3Interface.has_peer_device))
        print('%20s%s'%('lsc_enabled:',E3Interface.lsc_enabled))
        print('%20s%s'%('port_id:',E3Interface.port_id))
        print('%20s%s'%('peer_port_id:',E3Interface.peer_port_id))
        print('%20s%s'%('mac_addrs:',E3Interface.mac_addrs))
        print('%20s%s'%('input_node:',E3Interface.input_node))
        print('%20s%s'%('output_node:',E3Interface.output_node))
        print('%20s%s'%('_dummy0:',E3Interface._dummy0))
                
#return the list() of e3datapath interfaces 
#return the attached interface index
def attach_e3iface(dev_params,model,role):
    api_ret=c_uint64(0);
    _dev_parames=create_string_buffer(128)
    _dev_parames.value=dev_params.encode()
    _model=c_uint8(model)
    _role=c_uint8(role)
    _port=c_uint32(0)
    
    rc=clib.create_e3iface(byref(api_ret),_dev_parames,_model,_role,byref(_port))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception()
    return _port.value 
'''
after release a e3iface, you need to wait for a reasonable time to elapse before
you attach it again, the root cause is it need time to detach DPDK devce in RCU context which 
needs to be scheduled periodically
'''
def reclaim_e3iface(iface_id):
    api_ret=c_uint64(0);
    _port=c_uint16(iface_id)
    rc=clib.reclaim_e3iface(byref(api_ret),_port)
    if rc!=0:
        raise api_call_exception()
    return api_ret.value==0
def start_e3iface(iface_id):
    api_ret=c_uint64(0);
    _port=c_uint16(iface_id)
    rc=clib.update_e3fiace_status(byref(api_ret),_port,c_uint8(1))
    if rc!=0:
        raise api_call_exception()
def stop_e3iface(iface_id):
    api_ret=c_uint64(0);
    _port=c_uint16(iface_id)
    rc=clib.update_e3fiace_status(byref(api_ret),_port,c_uint8(0))
    if rc!=0:
        raise api_call_exception()

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
    
import time
if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    #print(attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT))
    #print(attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT))
    #E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT
    print(attach_e3iface('0000:00:08.0',E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT)) 
    #E3Interface().dump_definition()
    start_e3iface(1)
    #stop_e3iface(2)
    if_lst=get_e3iface_list()
    print('interface index list:',if_lst)
    
    for ifidx in if_lst:
        print(get_e3iface(ifidx))
    pass
    print(reclaim_e3iface(1))
