/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _E3IFACE_WRAPPER_H
#define _E3IFACE_WRAPPER_H
#include <e3interface.h>
int create_e3iface_with_slowpath(char * params,struct E3Interface_ops * ops,int *pport_id);
int release_e3iface_with_slowpath(int any_port_id);

int default_singly_queue_port_config(struct E3Interface * piface,struct rte_eth_conf * port_config);
#endif
