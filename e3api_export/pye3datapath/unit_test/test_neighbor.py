#
#Copyright (c) 2017 Jie Zheng
#
import unittest
from pye3datapath.e3iface import *
from e3net.common.e3log import get_e3loger
from e3net.common.e3config import add_config_file
from e3net.common.e3config import load_configs
from e3net.common.e3config import get_config
from pye3datapath.common.neighbor import *

class E3NeighborTest(unittest.TestCase):
    def setUp(self):
        #make sure no neighbors existing in datapath
        lst=list_neighbors()
        self.assertTrue(len(lst)==0)
    def tearDown(self):
        pass
    def test_neighbors_pool(self):
        #can not register duplicated neighbors 
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        n=get_neighbor(nidx)
        self.assertTrue(n.index==0)
        self.assertTrue(n.ref_cnt==0)
        self.assertTrue(n.is_valid==1)
        self.assertTrue(n._neighbor_ip_to_string()=='130.140.0.0')
        self.assertTrue(n._mac_to_string()=='00:01:02:03:04:05')
        try:
            register_neighbor('130.140.0.0','00:01:02:03:04:05')
            self.assertTrue(False)
        except:
            pass
        update_neighbor_mac('130.140.0.0','00:01:02:03:04:06')
        n=get_neighbor(nidx)
        self.assertTrue(n._mac_to_string()=='00:01:02:03:04:06')
        delete_neighbor(0)
        #smoke test for neighbors
        #the maximum neighbors is defined as MAX_COMMON_NEIGHBORS
        cnt=0
        for i in range(256):
            for j in range(256):
                if cnt==MAX_COMMON_NEIGHBORS:
                    break
                nidx=register_neighbor('130.140.%d.%d'%(i,j),'00:01:02:03:04:05')
                cnt=cnt+1
            if cnt==MAX_COMMON_NEIGHBORS:
                break
        try: 
            register_neighbor('131.141.0.0','00:01:02:03:04:05')
            self.assertTrue(False)
        except:
            self.assertTrue(True)
        lst=list_neighbors()
        self.assertTrue(len(lst)==MAX_COMMON_NEIGHBORS)
        cnt=0
        for n in lst:
            self.assertTrue(n.index==cnt)
            self.assertTrue(n.is_valid==1)
            self.assertTrue(n.ref_cnt==0)
            self.assertTrue(n._neighbor_ip_to_string()=='130.140.%d.%d'%(cnt>>8,cnt&0xff))
            cnt=cnt+1
        for n in lst:
            delete_neighbor(n.index)
        
