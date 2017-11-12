/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3-tlv.h>
#include <string.h>
#include <stdlib.h>

int message_builder_init(struct message_builder * builder,uint8_t * buffer,int length)
{
	if(length<MESSAGE_HDR_LENGTH)
		return -1;
	if(!buffer)
		return -2;
	builder->buffer=buffer;
	builder->buffer_len=length;
	builder->msg_hdr_ptr=(struct message_header*)builder->buffer;
	builder->msg_hdr_ptr->magic=MESSAGE_MAGIC;
	builder->msg_hdr_ptr->nr_tlvs=0;
	builder->msg_hdr_ptr->total_length=MESSAGE_HDR_LENGTH;
	builder->buffer_iptr=MESSAGE_HDR_LENGTH;
	builder->initialized=!0;
	return 0;
}

int message_builder_add_tlv(struct message_builder * builder,struct tlv_header * tlv,void * value)
{
	int needed_room;
	int available_room;
	struct tlv_header * tlv_tmp;
	
	if(!builder->initialized)
		return -1;

	needed_room=TLV_HDR_LENGTH+tlv->length;
	available_room=builder->buffer_len-builder->buffer_iptr;
	if(needed_room>available_room)
		return -2;

	tlv_tmp=(struct tlv_header*)(builder->buffer+builder->buffer_iptr);
	tlv_tmp->length=tlv->length;
	tlv_tmp->type=tlv->type;
	builder->buffer_iptr+=TLV_HDR_LENGTH;

	if(tlv->length)
		memcpy(builder->buffer_iptr+builder->buffer,value,tlv->length);
	builder->buffer_iptr+=tlv->length;

	builder->msg_hdr_ptr->total_length+=needed_room;
	builder->msg_hdr_ptr->nr_tlvs++;
	
	return 0;
}

int message_scan_tlv_entry(struct message_header * msg_hdr,
	uint8_t * entry_start,
	struct tlv_header * tlv,
	void**pvalue)
{
	int idx=0;
	int iptr=0;
	struct tlv_header * tlv_tmp;
	for(idx=0;idx<msg_hdr->nr_tlvs;idx++){
		tlv_tmp=(struct tlv_header*)(iptr+entry_start);
		if(tlv_tmp->type==tlv->type){
			tlv->length=tlv_tmp->length;
			*pvalue=tlv_tmp->value;
			return 0;
		}
		iptr+=TLV_HDR_LENGTH+tlv_tmp->length;
	}
	return -1;
}
struct tlv_major_index_base * allocate_tlv_major_index_base_from_glibc(void)
{
	struct tlv_major_index_base * base;
	base=malloc(sizeof(struct tlv_major_index_base)*MAJOR_TABLE_SIZE);
	if(base){
		memset(base,0x0,sizeof(struct tlv_major_index_base)*MAJOR_TABLE_SIZE);
	}
	return base;
}
int message_walk_through_tlv_entries(struct tlv_major_index_base * base,
	struct message_header * msg_hdr,
	uint8_t * entry_start,
	void * arg)
{
	struct tlv_callback_entry  * pentry;
	uint16_t base_index=MAJOR_TABLE_SIZE;
	uint16_t type_major;
	int idx,iptr,entry_index;
	struct tlv_header * tlv_tmp;
	
	for(idx=0,iptr=0;idx<msg_hdr->nr_tlvs;idx++){
		tlv_tmp=(struct tlv_header*)(entry_start+iptr);
		type_major=HIGH_UINT32(tlv_tmp->type);
		
	
		if(base_index==MAJOR_TABLE_SIZE)
			base_index=type_major;
		if(base_index!=type_major)/*all the TLVs must have same MAJOR type*/
			return -1;
		
		pentry=base[base_index].entries;
		if(!pentry)
			return -2;
		/*try to find the corresponding callback entry*/
		for(entry_index=0;pentry[entry_index].type;entry_index++){
			if(pentry[entry_index].type==tlv_tmp->type){
				if(pentry[entry_index].callback_func)
					pentry[entry_index].callback_func(tlv_tmp,tlv_tmp->value,arg);
			}
		}
		iptr+=TLV_HDR_LENGTH+tlv_tmp->length;
	}
	return 0;
}

int validate_tlv_encoding(void * buffer,int total_length)
{
	#define _(con) if(!(con)) goto error;
	
	struct message_header * msg_hdr=(struct message_header *)buffer;
	struct tlv_header * tlv=(struct tlv_header *)(MESSAGE_HDR_LENGTH+(uint8_t *)msg_hdr);
	int idx=0,iptr=0;
	_(total_length>=MESSAGE_HDR_LENGTH);
	_(msg_hdr->magic==MESSAGE_MAGIC);
	_(msg_hdr->total_length==total_length);
	
	for(idx=0,iptr=MESSAGE_HDR_LENGTH;idx<msg_hdr->nr_tlvs;idx++){
		iptr+=tlv->length+TLV_HDR_LENGTH;
		tlv=(struct tlv_header *)((tlv->length+TLV_HDR_LENGTH)+(uint8_t *)tlv);
	}
	_(iptr==total_length);
	return 1;
	#undef _
	error:
		return 0;
}
/*expand last tlv's length with given para*/
uint8_t * message_builder_expand_tlv(struct message_builder * builder,int len)
{
	uint8_t * new_start=NULL;
	int idx=0;
	int iptr;
	struct tlv_header * tlv=NULL;
	if(!builder->initialized)
		return NULL;
	if((builder->buffer_len-builder->buffer_iptr)<len)
		return NULL;
	for(idx=0,iptr=0;idx<builder->msg_hdr_ptr->nr_tlvs;idx++){
		tlv=(struct tlv_header*)(MESSAGE_HDR_LENGTH+builder->buffer+iptr);
		iptr+=TLV_HDR_LENGTH+tlv->length;
	}
	if(!tlv)
		return NULL;
	new_start=tlv->length+TLV_HDR_LENGTH+(uint8_t *)tlv;
	tlv->length+=len;
	builder->msg_hdr_ptr->total_length+=len;
	builder->buffer_iptr+=len;
	return new_start;
}
