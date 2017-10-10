#! /usr/bin/python3
from ctypes import *

CDLL("libzmq.so", mode=RTLD_GLOBAL)
clib=cdll.LoadLibrary("libe3api.so")#this is gonna be exported

class api_call_exception(Exception):
    def __init__(self,exp=None):
        self.explanation=exp
    def __str__(self):
        return repr(self.explanation)
 
class api_return_exception(Exception):
    def __init__(self,exp=None):
        self.explanation=exp
    def __str__(self):
        return repr(self.explanation)

def register_service_endpoint(endpoint):
    rc=clib.register_e3_api_client(c_char_p(endpoint.encode()+b'\x00'))
    if rc !=0:
        raise Exception('errors during registering service client')
        
if __name__=='__main__':
    register_service_endpoint('tcp://localhost:507')
    

