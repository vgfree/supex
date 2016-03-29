/**
 * Author       : chenzutao
 * Date         : 2015-10-27
 * Function     : filter GPS data
 **/
#ifndef __FILTER_H__
#define __FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"
#include "rr_cfg.h"
#include "libkv.h"
#include "gv_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int citycode_filter(kv_handler_t *handler, double lon, double lat);

int type_add(kv_handler_t *handler, long long IMEI, int type);

int status_add(kv_handler_t *handler, long long IMEI, int status);

int type_filter(kv_handler_t *handler, long long IMEI, int type);

int status_filter(kv_handler_t *handler, long long IMEI, int status);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __FILTER_H__ */

