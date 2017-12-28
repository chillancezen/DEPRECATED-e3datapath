#! /usr/bin/python3
import unittest
from pye3datapath.e3client import *

#from test_e3interface import E3InterfaceTest
#from test_neighbor import E3NeighborTest
#from test_nexthop import E3NexthopTest
#from test_eline_service import E3ELineTest
#from test_elan_service import E3ELanTest
#from test_csport import E3CsportTest
from test_cbport import E3CbportTest

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    unittest.main()
