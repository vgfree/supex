/*
 * Author       : chenzutao
 * Date         : 2015-1016
 * Function     : protobuf_decode.h
 */

#ifndef __PROTOBUF_ENCODE_H__
#define __PROTOBUF_ENCODE_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "decode_data.h"
#include "utils.h"

int cjson_to_gzpb(gps_data_t **gps_data, int len, int cnt, char **result);
#endif

