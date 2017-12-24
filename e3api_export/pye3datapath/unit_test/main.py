#! /usr/bin/python3
import unittest
from pye3datapath.e3client import *

from test_e3interface import E3InterfaceTest
from test_neighbor import E3NeighborTest
from test_nexthop import E3NexthopTest

if __name__=='__main__':
    register_service_endpoint('ipc:///var/run/e3datapath.sock')
    unittest.main()
