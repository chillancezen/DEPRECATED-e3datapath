import unittest
from pye3datapath.e3iface import *

class E3InterfaceTest(unittest.TestCase):
    def setUp(self):
        pass
    def tearDown(self):
        for ifidx in get_e3iface_list():
            stop_e3iface(ifidx)
            reclaim_e3iface(idx)
    def test_foo(self):
        print('hello world,foo')
