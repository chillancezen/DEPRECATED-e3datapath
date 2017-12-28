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
from pye3datapath.leaf.etherlan import *
from pye3datapath.leaf.cbport import *
import time

class E3CbportTest(unittest.TestCase):
    def setUp(self):
        self.assertTrue(register_ether_line_service()==0)
        self.assertTrue(register_ether_lan_service()==0)
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        nidx=register_nexthop(0,0)
        self.assertTrue(nidx==0)
        add_config_file('/etc/e3net/e3vswitch.ini')
        load_configs()
        self.iface_lst=list()
        lst=get_config(None,'test','pci_addr_lst').split(',')
        for pci in lst:
           self.iface_lst.append(pci.strip())
        self.assertTrue(len(self.iface_lst)>0)
        iface_idx=attach_e3iface(self.iface_lst[0],E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,True)
        self.assertTrue(iface_idx==0)
    def tearDown(self):
        reclaim_e3iface(0)
        time.sleep(3)
        delete_nexthop(0)
        delete_neighbor(0)
        delete_ether_lan_service(0)
        delete_ether_line_service(0)
    def test_cbport_pool(self):
        #now port 0 is the customer backbone port
        #make sure the label entry list is empty
        self.assertTrue(len(list_cbport_label_entries(0))==0)
        try:
            attach_cbport_to_eline(0,1,1)
            self.assertTrue(False)
        except:
            pass
        attach_cbport_to_eline(0,1,0)
        attach_cbport_to_eline(0,(1<<20)-1,0)
        try:
            attach_cbport_to_eline(0,(1<<20),0)
            self.assertTrue(False)
        except:
            pass
        self.assertTrue(len(list_cbport_label_entries(0))==2)
        eline=get_ether_line_service(0)
        self.assertTrue(eline.ref_cnt==1)
        attach_cbport_to_elan(0,1,0)
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.ref_cnt==1)

        attach_cbport_to_elan(0,(1<<20)-1,0)
        eline=get_ether_line_service(0)
        elan=get_ether_lan_service(0)
        self.assertTrue(eline.ref_cnt==0)
        self.assertTrue(elan.ref_cnt==1)
        
        detach_cbport(0,1)
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.ref_cnt==1)
        detach_cbport(0,(1<<20)-1)
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.ref_cnt==0)
        
         
