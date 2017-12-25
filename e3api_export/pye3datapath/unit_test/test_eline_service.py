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
from pye3datapath.leaf.etherline import *



class E3ELineTest(unittest.TestCase):
    def setUp(self):
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        nidx=register_nexthop(0,0)
        self.assertTrue(nidx==0)
    def tearDown(self):
        delete_nexthop(0)
        delete_neighbor(0)
    def test_eline_service_pool(self):
        #basic structure fields test
        eline_index=register_ether_line_service()
        self.assertTrue(eline_index==0)
        eline=get_ether_line_service(eline_index)
        self.assertTrue(eline.index==0)
        self.assertTrue(eline.is_valid==1)
        self.assertTrue(eline.is_cbp_ready==0)
        self.assertTrue(eline.is_csp_ready==0)
        #nhlfe setup test
        try:
            register_ether_line_nhlfe(0,1,100)
            self.assertTrue(False)
        except:
            pass
        register_ether_line_nhlfe(0,0,100)
        eline=get_ether_line_service(eline_index)
        self.assertTrue(eline.is_cbp_ready==1)
        self.assertTrue(eline.NHLFE==0)
        self.assertTrue(eline.label_to_push==100)
       
        delete_ether_line_nhlfe(0)
        eline=get_ether_line_service(eline_index)
        self.assertTrue(eline.is_cbp_ready==0)
        
        #port setup test
        register_ether_line_port(0,0,1000)
        eline=get_ether_line_service(eline_index)
        self.assertTrue(eline.is_csp_ready==1)
        self.assertTrue(eline.e3iface==0)
        self.assertTrue(eline.vlan_tci==1000)
        delete_ether_line_port(0)
        #clean the resource 
        delete_ether_line_service(0)
        
        #smoke test:MAX_E_LINE_SERVICES
        for i in range(MAX_E_LINE_SERVICES):
            idx=register_ether_line_service()
            self.assertTrue(idx==i)
        lst=list_ether_line_services()
        for eline_idx in lst:
            eline=get_ether_line_service(eline_idx) 
            self.assertTrue(eline.is_valid==1)
            self.assertTrue(eline.index==eline_idx)
        for eline_idx in lst:
            delete_ether_line_service(eline_idx)
        self.assertTrue(len(list_ether_line_services())==0)
        
