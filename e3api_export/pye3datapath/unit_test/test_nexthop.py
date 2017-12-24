#
#Copyright (c) 2017 Jie Zheng
#
import unittest
from pye3datapath.e3iface import *
from e3net.common.e3log import get_e3loger
from e3net.common.e3config import add_config_file
from e3net.common.e3config import load_configs
from e3net.common.e3config import get_config
from pye3datapath.common.nexthop import *
from pye3datapath.common.neighbor import *
class E3NexthopTest(unittest.TestCase):
    def setUp(self):
        lst=list_neighbors()
        self.assertTrue(len(lst)==0)
        lst=list_nexthops()
        self.assertTrue(len(lst)==0)

    def tearDown(self):
        self.assertTrue(len(list_neighbors())==0)
        self.assertTrue(len(list_nexthops())==0)
    
    def test_nexthops_pool(self):
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        #single nexthop test,invalid neighbor index
        try:
            nidx=register_nexthop(0,1)
            self.assertTrue(False)
        except:
            pass
        nidx=register_nexthop(0,0)
        self.assertTrue(nidx==0)
        #no duplicated nexthop entry is allowed
        try:
           register_nexthop(0,0)
           self.assertTrue(False)
        except:
            pass
        n=get_nexthop(0)
        self.assertTrue(n.index==0)
        self.assertTrue(n.ref_cnt==0)
        self.assertTrue(n.is_valid==1)
        self.assertTrue(n.local_e3iface==0)
        self.assertTrue(n.common_neighbor_index==0)
      
        #since neighbor #0 is referenced by nexthop #0,
        #it's not supposed to be deletable  
        neighbor=get_neighbor(0)
        self.assertTrue(neighbor.ref_cnt==1)
        try:
            delete_neighbor(0)
            self.assertTrue(False)
        except:
            pass
        #delete neighbor #0
        delete_nexthop(0)
        #smoke test
        for i in range(MAX_COMMON_NEXTHOPS):
            nidx=register_nexthop(i,0)
            self.assertTrue(nidx==i)
        lst=list_nexthops()
        idx=0
        for n in lst:
            self.assertTrue(n.index==idx)
            self.assertTrue(n.local_e3iface==idx)
            idx=idx+1
        for i in range(MAX_COMMON_NEXTHOPS):
            delete_nexthop(i)
        delete_neighbor(0)
            
