#! /usr/bin/python3
import tabulate
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint

def generate_memory_dump():
    api_ret=c_int64(0)
    rc=clib.dump_memory_structure(byref(api_ret))
    if rc!=0:
        raise api_call_exception()
    
if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    generate_memory_dump()

