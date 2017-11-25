#! /usr/bin/python3
from ctypes import *
from pye3datapath.e3client import clib
from pye3datapath.e3client import api_call_exception
from pye3datapath.e3client import api_return_exception
from pye3datapath.e3client import register_service_endpoint
import threading
from time import ctime,sleep

def foo(tid):
    while True:
        api_ret=c_uint64()
        rc=clib.e3datapath_version(byref(api_ret))
        if rc!=0:
            print(tid,'failed')
        else:
            print(tid,'succeeded')
    pass

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    threading.Thread(target=foo,args=(1,)).start()
    threading.Thread(target=foo,args=(2,)).start()
    threading.Thread(target=foo,args=(3,)).start()
    threading.Thread(target=foo,args=(4,)).start()
     
    sleep(100)   
