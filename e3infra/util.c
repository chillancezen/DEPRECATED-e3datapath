/*
*Copyright (c) 2016-2017 Jie Zheng
*/

#include <e3infra/include/util.h>
#include <stdio.h>

uint32_t _ip_string_to_u32_le(const char * ip_string)
{
	union{
		uint32_t ip_as_u32_le;
		uint8_t  ip_as_u8a[4];
	}ip_stub;
	int _ip_tmp[4];
	sscanf(ip_string,"%d.%d.%d.%d",&_ip_tmp[3],
					&_ip_tmp[2],
					&_ip_tmp[1],
					&_ip_tmp[0]);
	ip_stub.ip_as_u8a[0]=_ip_tmp[0];
	ip_stub.ip_as_u8a[1]=_ip_tmp[1];
	ip_stub.ip_as_u8a[2]=_ip_tmp[2];
	ip_stub.ip_as_u8a[3]=_ip_tmp[3];
	return ip_stub.ip_as_u32_le;
}
void _mac_string_to_byte_array(const char * mac_string,uint8_t  *mac_stub)
{
	int _mac_tmp[6];
	sscanf(mac_string,"%x:%x:%x:%x:%x:%x",
		&_mac_tmp[0],
		&_mac_tmp[1],
		&_mac_tmp[2],
		&_mac_tmp[3],
		&_mac_tmp[4],
		&_mac_tmp[5]);
	
	mac_stub[0]=_mac_tmp[0];
	mac_stub[1]=_mac_tmp[1];
	mac_stub[2]=_mac_tmp[2];
	mac_stub[3]=_mac_tmp[3];
	mac_stub[4]=_mac_tmp[4];
	mac_stub[5]=_mac_tmp[5];
}

