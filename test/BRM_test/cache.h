#ifndef         _CACHE_H_
#define         _CACHE_H_

#include <stdlib.h>
#include <stdio.h>

#include "stdbool.h"

#define  MACRO_START do {
#ifdef   MACRO_START
  #define  MACRO_END \
	}	     \
	while (0)
#endif

#define         _new(ptr)										   \
	MACRO_START											   \
		(ptr) = (__typeof__(ptr))malloc(sizeof(*ptr));						   \
	likely(ptr ? ptr : (errno = ENOMEM, x_printf(E, "there is no more memery, malloc failed"), NULL)); \
	MACRO_END

#define         _new_array(n, ptr)									   \
	MACRO_START											   \
	ptr = ((__typeof__(ptr))calloc((n), sizeof(*ptr)));						   \
	likely(ptr ? ptr : (errno = ENOMEM, x_printf(E, "there is no more memery, calloc failed"), NULL)); \
	MACRO_END

#define         _free(ptr)  \
	MACRO_START	    \
	if (likely(ptr)) {  \
		free(ptr);  \
		ptr = NULL; \
	}		    \
	MACRO_END

#define  CACHE_DEF_SIZE 4 * 1024
#define  CACHE_MAX_SIZE 10 * CACHE_DEF_SIZE

struct  netdata_cache
{
	int     offset;			// used cache size
	int     outsize;		// already sent to peer date size
	int     size;			// total cache size
	char    *data_buf;		// real cache address
	char    bufbase[CACHE_DEF_SIZE];
};

bool netdata_cache_init(struct netdata_cache *netdata);

void netdata_cache_free(struct netdata_cache *netdata);

void netdata_cache_reset(struct netdata_cache *netdata);

bool netdata_cache_add(struct netdata_cache *netdata, const char *buf, int size);
#endif	/* ifndef         _CACHE_H_ */

