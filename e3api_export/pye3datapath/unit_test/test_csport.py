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
from pye3datapath.leaf.csport import *
class E3CsportTest(unittest.TestCase):
    def setUp(self):
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        nidx=register_nexthop(0,0)
        self.assertTrue(nidx==0)
        #find the 1st PCI addr and create a csp port
        add_config_file('/etc/e3net/e3vswitch.ini')
        load_configs()
        self.iface_lst=list()
        lst=get_config(None,'test','pci_addr_lst').split(',')
        for pci in lst:
           self.iface_lst.append(pci.strip())
        self.assertTrue(len(self.iface_lst)>0)
        iface_idx=attach_e3iface(self.iface_lst[0],E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT,True)
        self.assertTrue(iface_idx==0)
        #setup the Ether services
        self.assertTrue(register_ether_line_service()==0)
        self.assertTrue(register_ether_lan_service()==0)
    def tearDown(self):
        delete_ether_line_service(0)
        delete_ether_lan_service(0)
        reclaim_e3iface(0)
        delete_nexthop(0)
        delete_neighbor(0)
    def test_csport_setting(self):
        try: 
            attach_csport_to_eline(0,100,1)
            self.assertTrue(False)
        except:
            pass
        attach_csport_to_eline(0,100,0)
        eline=get_ether_line_service(0)
        self.assertTrue(eline.is_valid==1)
        self.assertTrue(eline.ref_cnt==1)
        self.assertTrue(eline.is_csp_ready==1)
        self.assertTrue(eline.e3iface==0)
        self.assertTrue(eline.vlan_tci==100)

        #it should fail if another vlan entry(whether it's from the same csp port)
        try:
            attach_csport_to_eline(0,101,0)
            self.assertTrue(False)
        except:
            pass
        detach_csport(0,100)
        eline=get_ether_line_service(0)
        self.assertTrue(eline.is_csp_ready==0)
        self.assertTrue(eline.ref_cnt==0)
        #remap vlan:101 to E-LINE service 0
        attach_csport_to_eline(0,101,0)
        eline=get_ether_line_service(0)
        self.assertTrue(eline.ref_cnt==1)
        self.assertTrue(eline.is_valid==1)
        self.assertTrue(eline.is_csp_ready==1)
        self.assertTrue(eline.e3iface==0)
        self.assertTrue(eline.vlan_tci==101)
        #map vlan 101 to E-LAN service 0
        try:
            attach_csport_to_elan(0,101,1)
            self.assertTrue(False)
        except:
            pass
        attach_csport_to_elan(0,101,0)
        eline=get_ether_line_service(0)
        self.assertTrue(eline.ref_cnt==0)
        self.assertTrue(eline.is_csp_ready==0)
        
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.is_valid==1)
        self.assertTrue(elan.index==0)
        self.assertTrue(elan.ref_cnt==1)
        self.assertTrue(elan.nr_ports==1)
        self.assertTrue(elan.ports[0].is_valid==1)
        self.assertTrue(elan.ports[0].iface==0)
        self.assertTrue(elan.ports[0].vlan_tci==101)
        attach_csport_to_elan(0,101,0)
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.ref_cnt==1)
        self.assertTrue(elan.nr_ports==1)
        
        attach_csport_to_elan(0,100,0)
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.ref_cnt==2)
        self.assertTrue(elan.nr_ports==2)
        self.assertTrue(len(list_cspirt_dist_table(0))==2)
        detach_csport(0,101)
        detach_csport(0,100)
        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            attach_csport_to_elan(0,100+i,0)
        elan=get_ether_lan_service(0)
        self.assertTrue(len(list_cspirt_dist_table(0))==MAX_PORTS_IN_E_LAN_SERVICE)
        
        self.assertTrue(elan.ref_cnt==MAX_PORTS_IN_E_LAN_SERVICE)
        self.assertTrue(elan.nr_ports==MAX_PORTS_IN_E_LAN_SERVICE)

        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            self.assertTrue(elan.ports[i].is_valid==1)
            self.assertTrue(elan.ports[i].iface==0)
            self.assertTrue(elan.ports[i].vlan_tci==(100+i))
        
        try:
            attach_csport_to_elan(0,4094,0)
        except:
            pass
        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            detach_csport(0,100+i)
        
        self.assertTrue(len(list_cspirt_dist_table(0))==0)
