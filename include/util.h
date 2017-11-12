/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef _UTIL_H
#define _UTIL_H
#include <x86intrin.h>
#ifndef typeof
#define typeof __typeof__
#endif


#ifndef offsetof
#define offsetof(t, m) ((size_t) &((t *)0)->m)
#endif

#ifndef size_of_field
#define size_of_field(t,m)  ((size_t)sizeof((t *)0)->m)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({ \
		typeof(((type *)0)->member)(*__mptr) = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member)); })
#endif

#ifndef PREDICT_TRUE
#define PREDICT_TRUE(exp) __builtin_expect(!!(exp),1)
#endif

#ifndef PREDICT_FALSE
#define PREDICT_FALSE(exp) __builtin_expect(!!(exp),0)
#endif 

#define E3_MAX(a,b) (((a)>(b))?(a):(b))
#define E3_MIN(a,b) (((a)<(b))?(a):(b))

#define MAKE_UINT64(hi,lo)  ((((uint64_t)(hi))<<32)|(((uint64_t)(lo))&0xffffffff))
#define HIGH_UINT64(v) (((uint64_t)(v)>>32)&0xffffffff)
#define LOW_UINT64(v) (((uint64_t)(v))&0xffffffff)

#define MAKE_UINT32(hi,lo) ((((uint32_t)(hi))<<16)|(((uint32_t)(lo))&0xffff))
#define LOW_UINT32(v) ((uint16_t)((v)&0xffff))
#define HIGH_UINT32(v) ((uint16_t)(((v)>>16)&0xffff))

#define MAKE_UINT16(hi,lo) ((((uint16_t)(hi))<<8)|(((uint16_t)(lo))&0xff))
#define LOW_UINT16(v) ((uint8_t)((v)&0xff))
#define HIGH_UINT16(v) ((uint8_t)(((v)>>8)&0xff))

__attribute__((always_inline)) 
	static inline int is_ether_address_equal(void* a,void* b)
{
	return ((*(uint32_t*)(0+(uint8_t*)(a)))==(*(uint32_t*)(0+(uint8_t*)(b))))&& \
		((*(uint16_t*)(4+(uint8_t*)(a)))==(*(uint16_t*)(4+(uint8_t*)(b)))) ;
}
__attribute__((always_inline))
	static inline void copy_ether_address(void* dst,void * src)
{
	*(uint32_t*)(0+(uint8_t*)(dst))=*(uint32_t*)(0+(uint8_t*)(src)); 
	*(uint16_t*)(4+(uint8_t*)(dst))=*(uint16_t*)(4+(uint8_t*)(src)); 
}
#define SWAP_ORDER16(val) ((((val)<<8)&0xff00)|(((val)>>8)&0x00ff))

#define VNI_SWAP_ORDER(vni) (((((uint32_t)(vni))>>16)&0xff)| \
	(((uint32_t)(vni))&0xff00)| \
	((((uint32_t)(vni))<<16)&0xff0000))

#if defined(__SSE4_2__)
__attribute__((always_inline)) 
	static inline uint32_t crc32_hash(void *mem,int nr_words)
	{
		uint32_t crc_sum=0;
		
		int idx=0;
		for(idx=0;idx<nr_words;idx++){
			crc_sum=_mm_crc32_u16(crc_sum,*(nr_words+(uint16_t*)mem));
		}
		return crc_sum;
	}
#else
#error "not implemented"
#endif


#define VERSION_DWORD(major,minor,build) ((uint8_t)(build)|\
	(((minor)<<8)&0xff00)|\
	(((major)<<16)&0xff0000))


#define dump_field(s,f) { \
		int _offset=(int)offsetof(s,f);\
		int _size=(int)size_of_field(s,f);\
		int _gap=_offset-(last_offset+last_size);\
		printf("%20s (offset:%3d size:%3d prev_gap:%d)\n",#f,_offset,_size,_gap);\
		last_offset=_offset;\
		last_size=_size;\
}

enum e3error{
    E3_OK=0,
    E3_ERR_GENERIC=1,
    E3_ERR_ILLEGAL,
    E3_ERR_NOT_FOUND,
    E3_ERR_IN_USE,
    E3_ERR_OUT_OF_MEM,/*out of memory*/
    E3_ERR_OUT_OF_RES,/*out of other resource*/
};

#endif
