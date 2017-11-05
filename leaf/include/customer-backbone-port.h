#ifndef _CUSTOMER_BACKBONE_PORT_H
#define _CUSTOMER_BACKBONE_PORT_H
#include <leaf/include/leaf-label-fib.h>

struct cbp_private{	
	struct leaf_label_entry *label_base;
};

#endif