/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _MPLS_UTIL_H
#define _MPLS_UTIL_H
#include <stdint.h>
#define ETHER_PROTO_MPLS_UNICAST 0x4788

struct mpls_hdr{
	union{
		struct{
			uint8_t u80;
			uint8_t u81;
			uint8_t u82;
			uint8_t u83;
		};
		uint32_t dword0;
	};
}__attribute__((packed));


#define 	mpls_ttl(mpls) ((mpls)->u83)
#define set_mpls_ttl(mpls,ttl) (mpls)->u83=(uint8_t)(ttl)

#define 	  mpls_bottom(mpls) ((mpls)->u82&0x01)
#define   set_mpls_bottom(mpls) (mpls)->u82|=0x1
#define reset_mpls_bottom(mpls) (mpls)->u82&=0xfe

#define     mpls_exp(mpls) (((mpls)->u82>>1)&0x07)
#define set_mpls_exp(mpls,exp) {\
			(mpls)->u82&=0xf1;	\
			(mpls)->u82|=((exp)<<1)&0x0e;	\
}
#define mpls_label(mpls) ({\
	uint32_t _label=((mpls)->u82>>4)&0x0f; \
	_label|=((mpls)->u81<<4)&0xff0; \
	_label|=((mpls)->u80<<12)&0xff000; \
	_label;\
})
#define set_mpls_label(mpls,label){\
	(mpls)->u80=(label)>>12; \
	(mpls)->u81=(label)>>4; \
	(mpls)->u82&=0x0f; \
	(mpls)->u82|=((label)<<4)&0xf0; \
}

//#define IS_MAC_EQUAL(mac1,mac2) (!(((*(uint64_t*)(mac1))^(*(uint64_t*)(mac2)))&0xffffffffffff))
#define IS_MAC_EQUAL(mac1,mac2) ((mac1)[0]==(mac2)[0]&&\
	(mac1)[1]==(mac2)[1]&& \
	(mac1)[2]==(mac2)[2]&& \
	(mac1)[3]==(mac2)[3]&& \
	(mac1)[4]==(mac2)[4]&& \
	(mac1)[5]==(mac2)[5])
	
#endif