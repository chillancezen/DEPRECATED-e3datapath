#ifndef _E3_API_WRAPPER_H
#define _E3_API_WRAPPER_H
#include <e3-api.h>
#include <zmq.h>
#include <pthread.h>

/*system global definition*/

enum e3_tlv_major_type{
	e3_tlv_major_type_none=0,
	e3_tlv_major_type_api
};

enum e3_tlv_api_minor_type{
	e3_tlv_api_minor_type_none=0,
	e3_tlv_api_minor_type_begin,
	e3_tlv_api_minor_type_procname,
	e3_tlv_api_minor_type_retval,
	e3_tlv_api_minor_type_arg,
	e3_tlv_api_minor_type_status,
	/*0 indicates good,non-zero means bad result,
	and it's one-byte indicator*/
	e3_tlv_api_minor_type_end,
};
#define MAX_MSG_LENGTH (1024*16) 

struct e3_api_service{
	void * socket_handler;
	uint8_t recv_mbuf[MAX_MSG_LENGTH];
	uint8_t send_mbuf[MAX_MSG_LENGTH];
	int recv_length;
	int send_length;
	
	struct e3_api_declaration api_template;
	e3_type args_ptr_array[MAX_ARGUMENT_SUPPORTED+1];
	int arg_iptr;
	int error_cnt;
	
};
#define E3_API_CLIENT_TIMEOUT_MS 5000

struct e3_api_client{
	void * socket_handler;
	uint8_t recv_mbuf[MAX_MSG_LENGTH];
	uint8_t send_mbuf[MAX_MSG_LENGTH];
	int recv_length;
	int send_length;
	zmq_pollitem_t poll_items[1];/*currently ,only POLL ZMQ_POLLIN,
	because we need handle timeout event*/
	void ** para_output_list;/*the 0th is api_ret*/
	int nr_output_list;
	int output_list_indicator;
	int api_calling_status;

	struct e3_api_client * next_api_client;
	pthread_mutex_t client_guard;
	
};

struct e3_api_service * allocate_e3_api_service(char * service_endpoint_to_bind);
struct e3_api_client * allocate_e3_api_client(char * service_endpoint_to_connect);
int e3_api_service_try_to_poll_request(struct e3_api_service * service);
int e3_api_service_dispatch_apis(struct e3_api_service * service);
int e3_api_service_send_reponse(struct e3_api_service * service);
int encode_e3_api_request(uint8_t * buffer,
	int buffer_len,
	struct e3_api_declaration * api,
	e3_type * real_args);



#define declare_e3_api_client_base() extern struct e3_api_client * g_e3_api_client_ptr;
void publish_e3_api_client(struct e3_api_client * client);

struct e3_api_client * reference_e3_api_client(void);
void dereference_e3_api_client(struct e3_api_client * client);

int issue_e3_api_request(struct e3_api_client * client);

#endif