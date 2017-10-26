#ifndef _COMMON_NHLFE_H
#define _COMMON_NHLFE_H
#include <stdint.h>

struct common_neighbor{
	uint32_t neighbour_ip_as_le;
	uint16_t ref_cnt;
	uint16_t index;
	uint8_t  mac[6];
	uint8_t  is_valid;
	uint8_t  reserved0;
}__attribute__((packed));

#define MAX_COMMON_NEIGHBORS 2048

extern struct common_neighbor * neighbor_base;
int register_common_neighbor(struct common_neighbor * neighbor);

#endif