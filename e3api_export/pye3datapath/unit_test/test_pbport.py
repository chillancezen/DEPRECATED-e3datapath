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
from pye3datapath.spine.pbport import *
from pye3datapath.spine.mnexthop import *



class E3PbportTest(unittest.TestCase):
    def setUp(self):
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        nidx=register_nexthop(0,0)
        self.assertTrue(nidx==0)
        midx=register_mnexthop()
        self.assertTrue(midx==0)
        add_config_file('/etc/e3net/e3vswitch.ini')
        load_configs()
        self.iface_lst=list()
        lst=get_config(None,'test','pci_addr_lst').split(',')
        for pci in lst:
           self.iface_lst.append(pci.strip())
        self.assertTrue(len(self.iface_lst)>0)
        iface_idx=attach_e3iface(self.iface_lst[0],E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_PROVIDER_BACKBONE_PORT,True)
        self.assertTrue(iface_idx==0)
    def tearDown(self):
        reclaim_e3iface(0)
        time.sleep(3)
        delete_mnexthop(0)
        delete_nexthop(0)
        delete_neighbor(0)
    def test_pbport_pool(self):
        try:
            register_spine_label_entry(0,100,1,1000,1)
            self.assertTrue(False)
        except:
            pass
        try:
            register_spine_label_entry(0,1<<20,1,1000,0)
            self.assertTrue(False)
        except:
            pass
        self.assertTrue(len(list_spine_label_entry(0))==0)
        register_spine_label_entry(0,100,1,1000,0)
        n=get_nexthop(0)
        self.assertTrue(n.ref_cnt==1)
        register_spine_label_entry(0,(1<<20)-1,1,1023,0)
        self.assertTrue(len(list_spine_label_entry(0))==2)
        
        n=get_nexthop(0)
        self.assertTrue(n.ref_cnt==1)
        delete_spine_label_entry(0,100)
        n=get_nexthop(0)
        self.assertTrue(n.ref_cnt==1)
        delete_spine_label_entry(0,(1<<20)-1)
        n=get_nexthop(0)
        self.assertTrue(n.ref_cnt==0)
        
        register_spine_label_entry(0,10011,1,1000,0)
        register_spine_label_entry(0,10011,1,1000,0)
        n=get_nexthop(0)
        self.assertTrue(n.ref_cnt==1)
        
        register_spine_label_entry(0,10012,0,1023,0)
        m=get_mnexthop(0)
        self.assertTrue(m.ref_cnt==1)
        register_spine_label_entry(0,10011,0,1000,0)
        n=get_nexthop(0)
        m=get_mnexthop(0)
        self.assertTrue(n.ref_cnt==0)
        self.assertTrue(m.ref_cnt==1)

        register_spine_label_entry(0,10012,1,1023,0)
        n=get_nexthop(0)
        m=get_mnexthop(0)
        self.assertTrue(n.ref_cnt==1)
        self.assertTrue(m.ref_cnt==1)

        lst=list_spine_label_entry(0)
        for l in lst:
            delete_spine_label_entry(0,l.index)
        n=get_nexthop(0)
        m=get_mnexthop(0)
        self.assertTrue(n.ref_cnt==0)
        self.assertTrue(m.ref_cnt==0)
