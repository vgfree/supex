/**
 *
 **/

#ifndef __PARSER_MTTP_H__
#define __PARSER_MTTP_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mttp.h"
#include "libkv.h"
#include "cJSON.h"
#include "mttp_common.h"
#include "count.h"
#include "filter.h"

#define DATANUM         15
#define GZIP_BUFF_SIZE  1 << 11

typedef struct gps_single_s
{
	char            imei[32];
	unsigned int    lat;
	unsigned int    lon;
	int             speed;
	int             direction;
	unsigned int    gpstime;
} gps_single_t;

typedef struct gps_data_s
{
	unsigned int    size;
	gps_single_t    data[DATANUM];
} gps_data_t;

#ifdef __cplusplus
extern "C" {
#endif

int gzip_decompress(const char *src, int srcLen, char *dst);

int parsepb_to_struct(char *dst, int zdatalen, data_count_t *dt, gps_data_t *gps_dt);

int parse_body(const char *value, size_t len, data_count_t *dt, gps_data_t *gps_dt);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __PARSER_MTTP_H__ */

