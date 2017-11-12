/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#include <e3-api-wrapper.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void * zmq_context=NULL;
static struct tlv_major_index_base * api_tlv_index_base=NULL;
static struct tlv_major_index_base * client_api_tlv_index_base=NULL;
struct e3_api_client * g_e3_api_client_ptr=NULL;
pthread_mutex_t api_allocator_guard=PTHREAD_MUTEX_INITIALIZER;

int do_api_callback_func(struct e3_api_service * service)
{
	
	int idx=0;
	int nr_args=0;
	uint8_t * lptr;
	e3_type ret_val=0;
	uint8_t status=0;
	struct tlv_header tlv;
	struct message_builder builder;
	e3_type args_to_push_stack[MAX_ARGUMENT_SUPPORTED];
	struct e3_api_declaration * api=NULL;
	#define _(con) if(!(con)) goto error;
	service->send_length=0;
	_(api=search_e3_api_by_template(&service->api_template));
	
	_(!message_builder_init(&builder,service->send_mbuf,MAX_MSG_LENGTH));
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_begin);
	tlv.length=0;
	_(!message_builder_add_tlv(&builder,&tlv,NULL));
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_status);
	tlv.length=1;
	_(!message_builder_add_tlv(&builder,&tlv,&status));
	/*1. prepare the real parameters to pass to the callee*/
	nr_args=service->arg_iptr;
	for(idx=0;idx<nr_args;idx++){
		switch(service->api_template.args_desc[idx].behavior)
		{
			case e3_arg_behavior_input:
				args_to_push_stack[idx]=service->args_ptr_array[idx];
				break;
			case e3_arg_behavior_output:
				tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_arg);
				tlv.length=0;
				_(!message_builder_add_tlv(&builder,&tlv,NULL));
				_(lptr=message_builder_expand_tlv(&builder,service->api_template.args_desc[idx].len));
				args_to_push_stack[idx]=cast_to_e3_type(lptr);
				break;
			case e3_arg_behavior_input_and_output:
				tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_arg);
				tlv.length=0;
				_(!message_builder_add_tlv(&builder,&tlv,NULL));
				_(lptr=message_builder_expand_tlv(&builder,service->api_template.args_desc[idx].len));
				args_to_push_stack[idx]=cast_to_e3_type(lptr);
				memcpy(lptr,(void*)service->args_ptr_array[idx],service->api_template.args_desc[idx].len);
				break;
			default:
				_(0);
				break;
		}
	}
	/*2.call the registered api routine*/
	_(api->api_callback_func);
	switch(nr_args)
	{
		case 0:
			ret_val=api->api_callback_func(cast_to_e3_type(service));
			break;
		case 1:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0]);
			break;
		case 2:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0],
				args_to_push_stack[1]);
			break;
		case 3:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0],
				args_to_push_stack[1],
				args_to_push_stack[2]);
			break;
		case 4:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0],
				args_to_push_stack[1],
				args_to_push_stack[2],
				args_to_push_stack[3]);
			break;
		case 5:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0],
				args_to_push_stack[1],
				args_to_push_stack[2],
				args_to_push_stack[3],
				args_to_push_stack[4]);
			break;
		case 6:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0],
				args_to_push_stack[1],
				args_to_push_stack[2],
				args_to_push_stack[3],
				args_to_push_stack[4],
				args_to_push_stack[5]);
			break;
		case 7:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0],
				args_to_push_stack[1],
				args_to_push_stack[2],
				args_to_push_stack[3],
				args_to_push_stack[4],
				args_to_push_stack[5],
				args_to_push_stack[6]);
			break;
		case 8:
			ret_val=api->api_callback_func(cast_to_e3_type(service),
				args_to_push_stack[0],
				args_to_push_stack[1],
				args_to_push_stack[2],
				args_to_push_stack[3],
				args_to_push_stack[4],
				args_to_push_stack[5],
				args_to_push_stack[6],
				args_to_push_stack[7]);
			break;
		default:
			assert(({/*more parameters not supported,please refer to MAX_ARGUMENT_SUPPORTED*/0;}));
			break;
	}
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_retval);
	tlv.length=8;
	_(!message_builder_add_tlv(&builder,&tlv,&ret_val));
	
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_end);
	tlv.length=0;
	_(!message_builder_add_tlv(&builder,&tlv,NULL));
	service->send_length=builder.msg_hdr_ptr->total_length;
	#undef _
	return 0;
	error:
	/*re-init the builder and fill the buffer with error message*/
	service->send_length=0;
	if(!message_builder_init(&builder,service->send_mbuf,MAX_MSG_LENGTH))
	/*this may never happen after we make sure send_mbuf is large enough*/
		return -1;
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_begin);
	tlv.length=0;
	message_builder_add_tlv(&builder,&tlv,NULL);
	
	status=1;
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_status);
	tlv.length=1;
	message_builder_add_tlv(&builder,&tlv,&status);
	
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_begin);
	tlv.length=0;
	message_builder_add_tlv(&builder,&tlv,NULL);
	
	return -1;
}
/*if the minor type :e3_tlv_api_minor_type_arg,
the first two bytes are _type_,next two are _behavior_,
next two are length,then real value follows*/
void e3_api_tlv_callback_func(struct tlv_header * tlv,void * value,void * arg)
{
	struct e3_api_service * service=(struct e3_api_service*)arg;
	uint16_t type_major=tlv->type_major;
	uint16_t type_minor=tlv->type_minor;
	assert(type_major==e3_tlv_major_type_api);
	switch(type_minor)
	{
		case e3_tlv_api_minor_type_begin:
			service->arg_iptr=0;
			service->error_cnt=0;
			memset(service->args_ptr_array,0x0,sizeof(service->args_ptr_array));
			memset(&service->api_template,0x0,sizeof(service->api_template));
			break;
		case e3_tlv_api_minor_type_procname:
			service->api_template.api_name=(char*)value;
			break;
		case e3_tlv_api_minor_type_arg:
			if(service->arg_iptr==MAX_ARGUMENT_SUPPORTED)
				break;
			uint8_t * lptr=(uint8_t*)value;
			service->api_template.args_desc[service->arg_iptr].type=*(uint16_t*)(0+lptr);
			service->api_template.args_desc[service->arg_iptr].behavior=*(uint16_t*)(2+lptr);
			service->api_template.args_desc[service->arg_iptr].len=*(uint16_t*)(4+lptr);
			lptr+=6;
			switch(service->api_template.args_desc[service->arg_iptr].type)
			{
				case e3_arg_type_uint8_t:
					service->args_ptr_array[service->arg_iptr]=cast_to_e3_type(*(uint8_t*)lptr);
					break;
				case e3_arg_type_uint16_t:
					service->args_ptr_array[service->arg_iptr]=cast_to_e3_type(*(uint16_t*)lptr);
					break;
				case e3_arg_type_uint32_t:
					service->args_ptr_array[service->arg_iptr]=cast_to_e3_type(*(uint32_t*)lptr);
					break;
				case e3_arg_type_uint64_t:
					service->args_ptr_array[service->arg_iptr]=cast_to_e3_type(*(uint64_t*)lptr);
					break;
				case e3_arg_type_uint8_t_ptr:
					/*this makes sense only when the behavior is input(or _and_output)*/
					service->args_ptr_array[service->arg_iptr]=cast_to_e3_type((uint8_t*)lptr);
					break;
				default:
					service->error_cnt++;
					break;
				
			}
			service->arg_iptr++;
			break;
		case e3_tlv_api_minor_type_end:
			service->api_template.args_desc[service->arg_iptr].type=e3_arg_type_none;
			/*call registered real function now*/
			do_api_callback_func(service);
			break;
	}
}

void client_e3_api_tlv_callback_func(struct tlv_header * tlv,void * value,void * arg)
{
	struct e3_api_client * client=(struct e3_api_client*)arg;
	uint16_t type_major=tlv->type_major;
	uint16_t type_minor=tlv->type_minor;
	assert(type_major==e3_tlv_major_type_api);
	switch(type_minor)
	{
		case e3_tlv_api_minor_type_begin:
			client->output_list_indicator=1;
			client->api_calling_status=-1;
			break;
		case e3_tlv_api_minor_type_end:
			break;
		case e3_tlv_api_minor_type_status:
			assert(tlv->length==1);
			client->api_calling_status=*(uint8_t*)value;
			break;
		case e3_tlv_api_minor_type_arg:
			if(client->output_list_indicator<client->nr_output_list){
				memcpy(client->para_output_list[client->output_list_indicator],
					value,
					tlv->length);
				client->output_list_indicator++;
			}
			break;
		case e3_tlv_api_minor_type_retval:
			assert(tlv->length==8);
			*(uint64_t*)client->para_output_list[0]=*(uint64_t*)value;
			break;
		default:
			break;
	}
}
struct tlv_callback_entry api_tlv_entires[]={
		{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_procname),.callback_func=e3_api_tlv_callback_func},
		/*{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_retval),.callback_func=e3_api_tlv_callback_func},*/
		{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_arg),.callback_func=e3_api_tlv_callback_func},
		{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_begin),.callback_func=e3_api_tlv_callback_func},
		{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_end),.callback_func=e3_api_tlv_callback_func},
		{.type=0,.callback_func=0},
};

struct tlv_callback_entry client_api_tlv_entries[]={
	{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_arg),.callback_func=client_e3_api_tlv_callback_func},
	{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_retval),.callback_func=client_e3_api_tlv_callback_func},
	{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_status),.callback_func=client_e3_api_tlv_callback_func},
	{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_begin),.callback_func=client_e3_api_tlv_callback_func},
	{.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_end),.callback_func=client_e3_api_tlv_callback_func},
	{.type=0,.callback_func=0},
		
};
__attribute__((constructor)) 
	void e3_api_service_init(void)
{
	assert(zmq_context=zmq_ctx_new());
	assert(api_tlv_index_base=allocate_tlv_major_index_base_from_glibc());
	assert(client_api_tlv_index_base=allocate_tlv_major_index_base_from_glibc());
	api_tlv_index_base[e3_tlv_major_type_api].entries=api_tlv_entires;
	client_api_tlv_index_base[e3_tlv_major_type_api].entries=client_api_tlv_entries;
}

struct e3_api_service * allocate_e3_api_service(char * service_endpoint_to_bind)
{
	int rc;
	struct e3_api_service * service=NULL;
	service=malloc(sizeof(struct e3_api_service));
	if(!service)
		goto  error_dealloc;
	memset(service,0x0,sizeof(struct e3_api_service));
	service->socket_handler=zmq_socket(zmq_context,ZMQ_REP);
	if(!service->socket_handler)
		goto error_zmq;
	rc=zmq_bind(service->socket_handler,service_endpoint_to_bind);
	if(rc)
		goto error_zmq;
	
	return service;
	error_zmq:
		if(service->socket_handler)
			zmq_close(service->socket_handler);	
	error_dealloc:
		if(service)
			free(service);
	return NULL;
}

struct e3_api_client * allocate_e3_api_client(char * service_endpoint_to_connect)
{
	int rc;
	struct e3_api_client * client=malloc(sizeof(struct e3_api_client));
	if(!client){
		//printf("[api-client]:can not allocate client\n");
		goto  error_dealloc;
	}
	memset(client,0x0,sizeof(struct e3_api_client));
	client->socket_handler=zmq_socket(zmq_context,ZMQ_REQ);
	if(!client->socket_handler){
		//printf("[api-client]:can not create a zmq socket\n");
		goto error_zmq;
	}
	rc=zmq_connect(client->socket_handler,service_endpoint_to_connect);
	if(rc){
		//printf("[api-client]:can not connect client to endpoint:%s\n",service_endpoint_to_connect);
		goto error_zmq;
	}
	client->poll_items[0].socket=client->socket_handler;
	client->poll_items[0].fd=0;
	client->poll_items[0].events=ZMQ_POLLIN;
	client->poll_items[0].revents=0;
	client->next_api_client=NULL;
	pthread_mutex_init(&client->client_guard,NULL);
	return client;
	error_zmq:
		if(client->socket_handler)
			zmq_close(client->socket_handler);
	error_dealloc:
		if(client)
			free(client);
	return NULL;
}


int e3_api_service_try_to_poll_request(struct e3_api_service * service)
{
	int rc=zmq_recv(service->socket_handler,
		service->recv_mbuf,
		MAX_MSG_LENGTH,
		ZMQ_DONTWAIT);
	if(rc<0){
		/*here we do not check errno any more,
		bypass it quietly*/
		service->recv_length=0;
	}
	else 
		service->recv_length=rc;
	return service->recv_length;
}

int e3_api_service_dispatch_apis(struct e3_api_service * service)
{
	#define _(con) if(!(con)) goto ret;
	struct message_header * msg_hdr=(struct message_header*)service->recv_mbuf;
	_(validate_tlv_encoding(service->recv_mbuf,service->recv_length));
	_(msg_hdr->nr_tlvs);
	_(!message_walk_through_tlv_entries(api_tlv_index_base,
		msg_hdr,
		(uint8_t*)(msg_hdr+1),
		service));
	#undef _
	ret:
	return 0;
}
int e3_api_service_send_reponse(struct e3_api_service * service)
{
	service->send_length=service->send_length<0?0:service->send_length;
	zmq_send(service->socket_handler,
		service->send_mbuf,
		service->send_length,
		0);
	service->send_length=0;
	return 0;
}
int encode_e3_api_request(uint8_t * buffer,
	int buffer_len,
	struct e3_api_declaration * api,
	e3_type * real_args)
{
	int idx=0;
	int nr_args;
	uint8_t *  lptr;
	#define _(con) if(!(con)) goto error;
	struct message_builder builder;
	struct tlv_header  tlv;
	_(!message_builder_init(&builder,buffer,buffer_len));
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_begin);
	tlv.length=0;
	_(!message_builder_add_tlv(&builder,&tlv,NULL));
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_procname);
	tlv.length=strlen(api->api_name)+1;/*make sure NULL-terminator is included*/
	_(!message_builder_add_tlv(&builder,&tlv,api->api_name));
	nr_args=e3_api_nr_args(api);
	for(idx=0;idx<nr_args;idx++){
		tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_arg);
		tlv.length=0;
		_(!message_builder_add_tlv(&builder,&tlv,NULL));
		_(lptr=message_builder_expand_tlv(&builder,6));
		*(uint16_t*)(0+lptr)=api->args_desc[idx].type;
		*(uint16_t*)(2+lptr)=api->args_desc[idx].behavior;
		*(uint16_t*)(4+lptr)=api->args_desc[idx].len;
		switch(api->args_desc[idx].type)
		{
			case e3_arg_type_uint8_t:
				_(lptr=message_builder_expand_tlv(&builder,1));
				*(uint8_t*)lptr=e3_type_to_uint8_t(real_args[idx]);
				break;
			case e3_arg_type_uint16_t:
				_(lptr=message_builder_expand_tlv(&builder,2));
				*(uint16_t*)lptr=e3_type_to_uint16_t(real_args[idx]);
				break;
			case e3_arg_type_uint32_t:
				_(lptr=message_builder_expand_tlv(&builder,4));
				*(uint32_t*)lptr=e3_type_to_uint32_t(real_args[idx]);
				break;
			case e3_arg_type_uint64_t:
				_(lptr=message_builder_expand_tlv(&builder,8));
				*(uint64_t*)lptr=e3_type_to_uint64_t(real_args[idx]);
				break;
			case e3_arg_type_uint8_t_ptr:/*if the arg is input or input and output,copy the memory*/
				if(api->args_desc[idx].behavior!=e3_arg_behavior_output){
					_(lptr=message_builder_expand_tlv(&builder,api->args_desc[idx].len));
					memcpy((void*)lptr,(void*)real_args[idx],api->args_desc[idx].len);
				}
				break;
			default:
				_(0);
				break;
				
		}
	}
	tlv.type=MAKE_UINT32(e3_tlv_major_type_api,e3_tlv_api_minor_type_end);
	tlv.length=0;
	_(!message_builder_add_tlv(&builder,&tlv,NULL));
	#undef _
	return 0;
	error:
	return -1;
}

int issue_e3_api_request(struct e3_api_client * client)
{
	#define _(con) if(!(con)) goto error;
	struct message_header * msg_hdr=(struct message_header*)client->send_mbuf;
	client->send_length=msg_hdr->total_length;
	_(client->send_length==zmq_send(client->socket_handler,
		client->send_mbuf,
		client->send_length,
		0));
	zmq_poll(client->poll_items,1,E3_API_CLIENT_TIMEOUT_MS);
	_(client->poll_items[0].revents&ZMQ_POLLIN);
	client->recv_length=zmq_recv(client->socket_handler,
		client->recv_mbuf,
		MAX_MSG_LENGTH,
		0);
	_(client->recv_length>0);
	_(validate_tlv_encoding(client->recv_mbuf,client->recv_length));
	msg_hdr=(struct message_header*)client->recv_mbuf;
	_(!message_walk_through_tlv_entries(client_api_tlv_index_base,
		msg_hdr,
		(uint8_t*)(msg_hdr+1),
		(void*)client));
	_(!client->api_calling_status);
	#undef _
	return 0;
	error:
		return -1;
}

void publish_e3_api_client(struct e3_api_client * client)
{
	if(!g_e3_api_client_ptr){
		client->next_api_client=client;
		g_e3_api_client_ptr=client;
	}else{
		struct e3_api_client * lplast=g_e3_api_client_ptr;
		while(lplast->next_api_client!=g_e3_api_client_ptr)
			lplast=lplast->next_api_client;
		client->next_api_client=g_e3_api_client_ptr;
		lplast->next_api_client=client;
	}	
}
int register_e3_api_client(char * endpoint_address)
{
	struct e3_api_client * client=allocate_e3_api_client(endpoint_address);
	if(!client)
		return -1;
	publish_e3_api_client(client);
	return 0;
}
struct e3_api_client * reference_e3_api_client()
{

	static struct e3_api_client * last_client_used=NULL;
	struct e3_api_client * lptr=NULL;
	struct e3_api_client * lpfound=NULL;

	/*fix the bug that when no endpoint is registered, trying to reference
	crashes the program*/
	if(!g_e3_api_client_ptr)
		return NULL;
	pthread_mutex_lock(&api_allocator_guard);
	if(!last_client_used)
		last_client_used=g_e3_api_client_ptr;
	
	lptr=last_client_used;
	do{
		lptr=lptr->next_api_client;
		if(!pthread_mutex_trylock(&lptr->client_guard)){
			lpfound=lptr;
			last_client_used=lptr;
		}
	}while(lptr!=last_client_used);

	pthread_mutex_unlock(&api_allocator_guard);
	return lpfound;
}
void dereference_e3_api_client(struct e3_api_client * client)
{
	pthread_mutex_unlock(&client->client_guard);
}