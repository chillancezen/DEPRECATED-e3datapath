#ifndef _PROVIDER_BACKBONE_PORT_H
#define _PROVIDER_BACKBONE_PORT_H
#include <spine-label-fib.h>

/*per-port private*/

struct pbp_private{
	struct label_entry * label_base;
};
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
#endif
