/**
 *
 **/

#ifndef __PARSER_MTTP_H__
#define __PARSER_MTTP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils.h"
#include "mttp.h"
#include "gv_common.h"
#include "libkv.h"
#include "cJSON.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "mirrtalk-transfer-data.pb.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message_lite.h"

#define DATANUM         15
#define GZIP_BUFF_SIZE  1 << 11

typedef struct gps_single_s
{
	char            *imei;
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

int gzip_decompress(const char *src, int srcLen, char *dst);

int parsepb_to_struct(char *dst, int zdatalen, void **gps_dt);

int parser_mttp(struct data_node *p_node);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __PARSER_MTTP_H__ */

