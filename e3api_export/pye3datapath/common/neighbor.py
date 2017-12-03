#! /usr/bin/python3
from ctypes import *
from pye3datapath.e3util import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

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
    
    rc=clib.e3net_api_register_common_neighbor(byref(api_ret),ip_string,mac_string)
    if rc!=0:
        raise api_call_exception(rc)
    if api_ret.value<0:
        api_return_exception('registering common neighbor fails with api_ret:%x'%(api_ret.value))
    return api_ret.value

    
if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e4datapath.sock')
    print(register_neighbor('2.3.2.232','09:22:2:3:43:2'))
