#pragma once

#include "http_api/http_status.h"
#include "redis_api/redis_status.h"
#include "mttp_api/mttp_status.h"
#include "mfptp_api/mfptp_parse.h"

/** async protocol callback prototype */
typedef int (PROTO_CALL_BACK)(void *parse, ...);

/*
 * = 0;PROTO_STAT_OMIT
 * < 0;PROTO_STAT_FAIL
 * > 0;PROTO_STAT_DONE
 */
int http_proto_init(struct http_parse_info *info, char *const *data, unsigned const *size);

int http_proto_free(struct http_parse_info *info);

int http_proto_resp(struct http_parse_info *info);

int http_proto_reqt(struct http_parse_info *info);

struct http_parse_info *http_proto_dump(struct http_parse_info *src);

int http_proto_good(struct http_parse_info *src);


int redis_proto_init(struct redis_parse_info *info, char *const *data, unsigned const *size);

int redis_proto_free(struct redis_parse_info *info);

int redis_proto_resp(struct redis_parse_info *info);

int redis_proto_reqt(struct redis_parse_info *info);

struct redis_parse_info *redis_proto_dump(struct redis_parse_info *src);

int redis_proto_good(struct redis_parse_info *src);



int mttp_proto_init(struct mttp_parse_info *info, char *const *data, unsigned const *size);

int mttp_proto_free(struct mttp_parse_info *info);

int mttp_proto_reqt(struct mttp_parse_info *info);

#define mttp_proto_resp mttp_proto_reqt

struct mttp_parse_info *mttp_proto_dump(struct mttp_parse_info *src);

int mttp_proto_good(struct mttp_parse_info *src);



int mfptp_proto_init(struct mfptp_parse_info *info, char *const *data, unsigned const *size);

int mfptp_proto_free(struct mfptp_parse_info *info);

int mfptp_proto_reqt(struct mfptp_parse_info *info);

#define mfptp_proto_resp mfptp_proto_reqt

struct mfptp_parse_info *mfptp_proto_dump(struct mfptp_parse_info *src);

int mfptp_proto_good(struct mfptp_parse_info *src);
