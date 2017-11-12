/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _LEAF_NHLFE_H
#define _LEAF_NHLFE_H
#include <stdint.h>

struct leaf_neighbor{
	uint32_t neighbour_ip_as_le;
	uint16_t ref_cnt;
	uint16_t index;
	uint8_t  mac[6];
	uint8_t  is_valid;
	uint8_t  reserved0;
}__attribute__((packed));


#endif