/*
*Copyright (c) 2016-2017 Jie Zheng
*/
/**
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `ini.c` for details.
 */

#ifndef _E3_INI_H
#define _E3_INI_H

typedef struct ini_t ini_t;

ini_t*      ini_load(const char *filename);
void        ini_free(ini_t *ini);
const char* ini_get(ini_t *ini, const char *section, const char *key);
int         ini_sget(ini_t *ini, const char *section, const char *key, const char *scanfmt, void *dst);
#endif


