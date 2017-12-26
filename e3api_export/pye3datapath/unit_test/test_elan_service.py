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
from pye3datapath.leaf.etherlan import *


class E3ELanTest(unittest.TestCase):
    def setUp(self):
        nidx=register_neighbor('130.140.0.0','00:01:02:03:04:05')
        self.assertTrue(nidx==0)
        nidx=register_nexthop(0,0)
        self.assertTrue(nidx==0)
    def tearDown(self):
        delete_nexthop(0)
        delete_neighbor(0)
    def test_elan_service_pool(self):
        #fields of E-LAN test
        elan_idx=register_ether_lan_service()
        self.assertTrue(elan_idx==0)
        elan=get_ether_lan_service(elan_idx)
        self.assertTrue(elan.is_valid==1)
        self.assertTrue(elan.is_releasing==0)
        self.assertTrue(elan.index==0)
        self.assertTrue(elan.ref_cnt==0)
        self.assertTrue(elan.nr_ports==0)
        self.assertTrue(elan.nr_nhlfes==0)
        self.assertTrue(elan.multicast_NHLFE==-1)
        self.assertTrue(elan.fib_base!=0)
        
        #port entries test
        port_idx=register_ether_lan_port(0,0,100)
        try:
            #do duplicated port entry is allowed
            register_ether_lan_port(0,0,100)
            self.assertTrue(False)
        except:
            pass
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.nr_ports==1)
        self.assertTrue(elan.ports[0].is_valid==1)
        self.assertTrue(elan.ports[0].iface==0)
        self.assertTrue(elan.ports[0].vlan_tci==100)         
        delete_ether_lan_port(0,0) 
    
        #enumerate the port entries
        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            port_idx=register_ether_lan_port(0,0,i+100)
            self.assertTrue(port_idx==i)
        elan=get_ether_lan_service(0)
        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            self.assertTrue(elan.ports[i].is_valid==1)
            self.assertTrue(elan.ports[i].iface==0)
            self.assertTrue(elan.ports[i].vlan_tci==(i+100))
        self.assertTrue(elan.nr_ports==MAX_PORTS_IN_E_LAN_SERVICE)
        for i in range(MAX_PORTS_IN_E_LAN_SERVICE):
            delete_ether_lan_port(0,i)
        #nhlfe entries test
        try:
            register_ether_lan_nhlfe(0,1,1001)
            self.assertTrue(False)
        except:
            pass
        n_idx=register_ether_lan_nhlfe(0,0,1001)
        try:
            register_ether_lan_nhlfe(0,0,1001)
            self.assertTrue(False)
        except:
            pass
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.nr_nhlfes==1)
        self.assertTrue(elan.nhlfes[0].is_valid==1)
        self.assertTrue(elan.nhlfes[0].NHLFE==0)
        self.assertTrue(elan.nhlfes[0].label_to_push==1001)
        delete_ether_lan_nhlfe(0,0)
        
        #enumerate the nhlfe entries
        for i in range(MAX_NHLFE_IN_E_LAN_SERVICE):
            n_idx=register_ether_lan_nhlfe(0,0,i+1000)
            self.assertTrue(n_idx==i)
        elan=get_ether_lan_service(0)
        for i in range(MAX_NHLFE_IN_E_LAN_SERVICE):
            self.assertTrue(elan.nhlfes[i].is_valid==1)
            self.assertTrue(elan.nhlfes[i].NHLFE==0)
            self.assertTrue(elan.nhlfes[i].label_to_push==(i+1000))
        self.assertTrue(elan.nr_nhlfes==MAX_NHLFE_IN_E_LAN_SERVICE)
        for i in range(MAX_NHLFE_IN_E_LAN_SERVICE):
            delete_ether_lan_nhlfe(0,i)
        #multicast NHLFE entry test
        set_ether_lan_multicast_fwd_entry(0,0,11110)
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.multicast_NHLFE==0)
        self.assertTrue(elan.multicast_label==11110)
        reset_ether_lan_multicast_fwd_entry(0)
        elan=get_ether_lan_service(0)
        self.assertTrue(elan.multicast_NHLFE==-1)
        delete_ether_lan_service(0)
            
        #enumerate e-lan services
        for i in range(MAX_E_LAN_SERVICES):
            n_idx=register_ether_lan_service()
            self.assertTrue(n_idx==i)
        lst=list_ether_lan_services()
        for l in lst:
            elan=get_ether_lan_service(l)
            self.assertTrue(elan.is_valid==1)
            self.assertTrue(elan.is_releasing==0)
            self.assertTrue(elan.index==l)
            self.assertTrue(elan.ref_cnt==0)
        for l in lst:
            delete_ether_lan_service(l)
        self.assertTrue(len(list_ether_lan_services())==0)
        
