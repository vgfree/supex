/**
 * Author       : chenzutao
 * Date         : 2016-01-27
 * Function     : redis operation by C language
 **/

#ifndef _REDIS_OPERATION_H
#define _REDIS_OPERATION_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "hiredis.h"

#ifdef __cplusplus
extern "C" {
#endif

redisContext *redis_connect(const char *host, short port);

int redis_set(redisContext *conn, const char *key, const char *value);

int redis_ping(redisContext *conn);

long get_from_nodeID_by_roadID(redisContext *conn, long roadID);

long get_end_nodeID_by_roadID(redisContext *conn, long roadID);

redisReply *get_import_roadID(redisContext *conn, long nodeID);

redisReply *get_export_roadID(redisContext *conn, long nodeID);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef _REDIS_OPERATION_H */

