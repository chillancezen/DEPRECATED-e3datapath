from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

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

if __name__=='__main__':
    register_service_endpoint('tcp://localhost:507')
    print(register_neighbor('13.140.150.1','0:01:23:24:35:45'))
    print(register_neighbor('130.140.150.1','01:01:23:24:35:45'))
