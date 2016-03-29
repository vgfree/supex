#pragma once

#include <time.h>

#include "redis_parse.h"

#include "timport_cfg.h"

#include "timport_task.h"

int64_t get_stime(int32_t fd);

int32_t set_stime(int32_t fd, int64_t t);

inline int64_t get_file_size(const char *path);

inline int write_file(int32_t fd, const char *buf, int32_t count);

int sort_array(struct redis_reply *reply);

int unique_array(struct redis_reply *reply);

int fmt_task_key(timport_task_t *p_ttask);

int get_to_proto(char **proto, timport_key_t *p_tkey, char *param, char *ftime);

int set_to_proto(char **proto, timport_key_t *p_tkey, char *param, char *ftime, char *data, size_t size);

int expire_to_proto(char **proto, timport_key_t *p_tkey, char *param, char *ftime, int expire_time);

char *get_ftime(time_t tmstamp, tkey_interval_e interval);

int set_to_string(struct redis_reply **dst, struct redis_reply *src, int ignore_error);

int merge_child_sets(timport_task_t *p_ttask);

unsigned int custom_hash_dist(const void *data, unsigned int len, int srv_num, int suffix);

struct redis_reply      *redis_reply_dup(struct redis_reply *src);

