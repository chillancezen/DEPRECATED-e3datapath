#
#Copyright (c) 2017 Jie Zheng
#
import unittest
from pye3datapath.e3iface import *
from e3net.common.e3log import get_e3loger
from e3net.common.e3config import add_config_file
from e3net.common.e3config import load_configs
from e3net.common.e3config import get_config
from pye3datapath.e3iface import *
import time

logger=get_e3loger('e3datapath.api.unittest')

roles=[E3IFACE_ROLE_PROVIDER_BACKBONE_PORT,
        E3IFACE_ROLE_CUSTOMER_BACKBONE_FACING_PORT,
        E3IFACE_ROLE_CUSTOMER_USER_FACING_PORT]

class E3InterfaceTest(unittest.TestCase):
    def setUp(self):
        add_config_file('/etc/e3net/e3vswitch.ini')
        load_configs()
        self.iface_lst=list()
        lst=get_config(None,'test','pci_addr_lst').split(',')
        for pci in lst:
           self.iface_lst.append(pci.strip())
        self.assertTrue(len(self.iface_lst)>0)
         
    def tearDown(self):
        for ifidx in get_e3iface_list():
            stop_e3iface(ifidx)
            reclaim_e3iface(ifidx)
    def test_interface_setup(self):
        for role in roles:
            iface_idx=attach_e3iface(self.iface_lst[0],E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,role,True)
            logger.info('create interface with parameters:[%s,%d,%d,True] as %d'%(self.iface_lst[0],
                        E3IFACE_MODEL_GENERIC_SINGLY_QUEUE,
                        role,
                        iface_idx))
            iface=get_e3iface(iface_idx)
            logger.info('e3interface data:%s'%(str(iface)))
            self.assertTrue(iface.port_id==iface_idx)
            self.assertTrue(iface.hwiface_model==E3IFACE_MODEL_GENERIC_SINGLY_QUEUE)
            self.assertTrue(iface.hwiface_role==role)
            self.assertTrue(iface.iface_status==0)
            start_e3iface(iface_idx)
            iface=get_e3iface(iface_idx)
            self.assertTrue(iface.iface_status==1)
            self.assertTrue(iface.has_peer_device==1)
            self.assertTrue(iface.under_releasing==0)
            self.assertTrue(iface.nr_queues==1)
            reclaim_e3iface(iface_idx)
            #it's supposed to be not indexable
            try:
                get_e3iface(iface_idx)
                self.assertTrue(False)
            except:
                pass
            #wait for 3 seconds until the pci resource is already released
            #the interface resource must be released in RCU context asynchronously
            #and this is supposed to be kind of deficiency
            time.sleep(3)
