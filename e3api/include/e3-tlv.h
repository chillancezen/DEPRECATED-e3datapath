#ifndef E3_TLV_H
#define E3_TLV_H
#include <inttypes.h>


#define MAKE_UINT32(hi,lo) ((((uint32_t)(hi))<<16)|(((uint32_t)(lo))&0xffff))
#define LOW_UINT32(v) ((uint16_t)((v)&0xffff))
#define HIGH_UINT32(v) ((uint16_t)(((v)>>16)&0xffff))



struct tlv_header{
	union{
		uint32_t type;
		struct{
			uint16_t type_minor;
			uint16_t type_major;
		};
	};
	uint16_t length;
	uint8_t value[0];
}__attribute__((packed));
#define TLV_HDR_LENGTH (sizeof(struct tlv_header))

#define MESSAGE_MAGIC 0x55aa
struct message_header{
	uint16_t magic;
	uint16_t nr_tlvs;
	uint32_t total_length;
}__attribute__((packed));
#define MESSAGE_HDR_LENGTH (sizeof(struct message_header))

struct message_builder{
	uint8_t initialized;
	uint8_t * buffer;
	uint32_t buffer_len;
	uint32_t buffer_iptr;
	struct message_header * msg_hdr_ptr;
};

#define MAJOR_TABLE_SIZE 1024
#define MAX_TLV_MAJOR_NR (MAJOR_TABLE_SIZE-1)

struct tlv_callback_entry{
	void (*callback_func)(struct tlv_header* ,void*,void *);
	uint32_t type;
	uint32_t reserved;
};
struct tlv_major_index_base{
	struct tlv_callback_entry * entries;
};
struct tlv_major_index_base * allocate_tlv_major_index_base_from_glibc(void);


int message_builder_init(struct message_builder * builder,uint8_t * buffer,int length);
int message_builder_add_tlv(struct message_builder * builder,struct tlv_header * tlv,void * value);
int message_scan_tlv_entry(struct message_header * msg_hdr,
	uint8_t * entry_start,
	struct tlv_header * tlv,
	void**pvalue);
int message_walk_through_tlv_entries(struct tlv_major_index_base * base,
	struct message_header * msg_hdr,
	uint8_t * entry_start,
	void * arg);
uint8_t * message_builder_expand_tlv(struct message_builder * builder,int len);

int validate_tlv_encoding(void * buffer,int total_length);






#endif
