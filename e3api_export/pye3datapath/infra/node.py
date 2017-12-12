#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint
'''
enum node_type{
    node_type_misc=0,
    node_type_input=1,
    node_type_output=2,
    node_type_worker=3
};
'''
node_type={0:'node_type_misc',
    1:'node_type_input',
    2:'node_type_output',
    3:'node_type_worker'}

class node(Structure):
    _pack_=1
    _fields_=[('name',c_char*64),
                ('nr_rx_packets',c_int64),
                ('nr_tx_packets',c_int64),
                ('nr_drop_packets',c_int64),
                ('node_index',c_int16),
                ('lcore_id',c_int8),
                ('burst_size',c_int8),
                ('reserved0',c_uint8*4),
                ('node_ring',c_uint64),
                ('next',c_uint64),
                ('node_priv',c_uint64),
                ('reserved1',c_uint8*552),
                ('node_type',c_uint32),
                ('reserved3',c_uint8*28)]

    def dump_definition(self):
        print('size of node is:',sizeof(node))
        print(node.name,'name')
        print(node.nr_rx_packets,'nr_rx_packets')
        print(node.nr_tx_packets,'nr_tx_packets')
        print(node.nr_drop_packets,'nr_drop_packets')
        print(node.node_index,'node_index')
        print(node.lcore_id,'lcore_id')
        print(node.burst_size,'burst_size')
        print(node.reserved0,'reserved0')
        print(node.node_ring,'node_ring')
        print(node.next,'next')
        print(node.node_priv,'node_priv')
        print(node.reserved1,'reserved1')
        print(node.node_type,'node_type')
        print(node.reserved3,'reserved3')
    def __str__(self):
        ret=dict()
        ret['name']=str(self.name)
        ret['nr_rx_packets']=self.nr_rx_packets
        ret['nr_tx_packets']=self.nr_tx_packets
        ret['nr_drop_packets']=self.nr_drop_packets
        ret['node_index']=self.node_index
        ret['lcore_id']=self.lcore_id
        ret['burst_size']=self.burst_size
        ret['node_ring']='0x%x'%(self.node_ring)
        ret['next']='0x%x'%(self.next)
        ret['node_priv']='0x%x'%(self.node_priv)
        ret['node_type']=node_type[self.node_type]
        return str(ret)

def get_node(node_index):
    api_ret=c_int64(0)
    _node_index=c_int16(node_index)
    n=node()
    rc=clib.infra_api_get_node(byref(api_ret),_node_index,byref(n))
    if rc!=0:
        raise api_call_exception()
    if api_ret.value!=0:
        raise api_return_exception('api_ret:%x'%(api_ret.value))
    return n

MAX_NR_NODES=1024


def list_nodes():
    api_ret=c_int64(0)
    class A(Structure):
        _pack_=1
        _fields_=[('a',c_int8*MAX_NR_NODES)]
    a=A()
    lst=list()
    rc=clib.infra_api_list_nodes(byref(api_ret),byref(a))
    if rc!=0:
        raise api_call_exception()
    for i in range(MAX_NR_NODES):
        if a.a[i]!=0:
            lst.append(i)
    return lst
def tabulate_nodes():
    node_lst=list_nodes()
    table=list()
    for n in node_lst:
        node=get_node(n)
        table.append([node.node_index,
            node.name,
            node_type[node.node_type],
            node.lcore_id,
            '0x%x'%(node.node_priv),
            '0x%x'%(node.node_ring)])
    print(tabulate.tabulate(table,['index','node name','node type','lcore id','node private','node ring'],tablefmt='psql'))
if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    tabulate_nodes()
