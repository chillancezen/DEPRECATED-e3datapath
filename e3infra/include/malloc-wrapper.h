/*
*Copyright (c) 2016-2017 Jie Zheng
*/
#ifndef __MALLOC_WRAPPER_H
#define __MALLOC_WRAPPER_H
#include <stdio.h>
void * RTE_ZMALLOC(const char *type, size_t size, unsigned align);
void * 	RTE_ZMALLOC_SOCKET(const char *type, size_t size, unsigned align, int socket);
void RTE_FREE(void *ptr);
#endif
