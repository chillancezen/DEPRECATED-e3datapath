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
from pye3datapath.spine.mnexthop import *


class E3MulticastNexthopsTest(unittest.TestCase):
    def setUp(self):
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        nidx=register_nexthop(0,0)
        self.assertTrue(nidx==0)
    def tearDown(self):
        delete_nexthop(0)
        delete_neighbor(0)
    def test_multicast_nexthops_pool(self):
        midx=register_mnexthop()
        self.assertTrue(midx==0)
        m=get_mnexthop(0)
        self.assertTrue(m.index==0)
        self.assertTrue(m.is_valid==1)
        self.assertTrue(m.ref_cnt==0)
        self.assertTrue(m.nr_hops==0)
        for i in range(MAX_HOPS_IN_MULTICAST_GROUP):
            self.assertTrue(m.nexthops[i].is_valid==0)
        self.assertTrue(len(list_mnexthops())==1)
        idx=register_nexthop_in_mnexthops(0,0,100)
        self.assertTrue(idx==0)
        m=get_mnexthop(0)
        self.assertTrue(m.ref_cnt==0)
        self.assertTrue(m.nr_hops==1)
        self.assertTrue(m.nexthops[0].is_valid==1)
        self.assertTrue(m.nexthops[0].next_hop==0)
        self.assertTrue(m.nexthops[0].label_to_push==100)
        delete_nexthop_in_mnexthops(0,0,100)
        
        m=get_mnexthop(0)
        self.assertTrue(m.nr_hops==0)
        self.assertTrue(m.nexthops[0].is_valid==0)
        #enumerate the multicast nexthop entry
        for i in range(MAX_HOPS_IN_MULTICAST_GROUP):
            inner_idx=register_nexthop_in_mnexthops(0,0,i+1)
            self.assertTrue(inner_idx==i)
        try:
            register_nexthop_in_mnexthops(0,0,10000)
            self.assertTrue(False)
        except:
            pass
        m=get_mnexthop(0)
        self.assertTrue(m.nr_hops==MAX_HOPS_IN_MULTICAST_GROUP)
        for i in range(MAX_HOPS_IN_MULTICAST_GROUP):
            self.assertTrue(m.nexthops[i].is_valid==1)
            self.assertTrue(m.nexthops[i].next_hop==0)
            self.assertTrue(m.nexthops[i].label_to_push==(i+1))
        for i in range(MAX_HOPS_IN_MULTICAST_GROUP):
            delete_nexthop_in_mnexthops(0,0,i+1)
        m=get_mnexthop(0)
        self.assertTrue(m.nr_hops==0)
        delete_mnexthop(0)
        #enumerate multicast entries
        for i in range(MAX_MULTICAST_NEXT_HOPS):
            midx=register_mnexthop()
            self.assertTrue(midx==i)
        self.assertTrue(len(list_mnexthops())==MAX_MULTICAST_NEXT_HOPS)
        try:
            register_mnexthop()
            self.assertTrue(False)
        except:
            pass
        for i in range(MAX_MULTICAST_NEXT_HOPS):
            delete_mnexthop(i)
