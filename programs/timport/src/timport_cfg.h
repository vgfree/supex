#pragma once

#include "supex.h"

#include "redis_parse.h"

typedef enum
{
	TKEY_TYPE_UNKNOWN = 0,
	TKEY_TYPE_WHOLE_INDEX,
	TKEY_TYPE_ALONE_INDEX,
	TKEY_TYPE_KEYS_INDEX,
	TKEY_TYPE_SET_VAL,
	TKEY_TYPE_STRING_VAL
} tkey_type_e;

typedef enum
{
	TKEY_INTERVAL_UNKNOWN = 0,
	TKEY_INTERVAL_TEN_MIN,
	TKEY_INTERVAL_ONE_HOUR
} tkey_interval_e;

struct timport_key;

typedef void (*TKEY_RESULT_FILTER)(struct timport_key *p_tkey, struct redis_reply **reply);
typedef int (*TKEY_KEY_FILTER)(struct timport_key *p_tkey, char **dst, char *src, int src_len);
typedef int (*TKEY_HASH_FILTER)(struct timport_key *p_tkey, char *data, int len);
typedef int (*TKEY_REDIS_FILTER)(struct timport_key *p_tkey, char *data, int len, int srv_num);

typedef struct redis_link
{
	char    *host;
	int     port;
	int     idx;
} redis_link_t;

typedef struct tsdb_kset
{
	int             s_key;
	int             e_key;
	int             dn_cnt;
	redis_link_t    data_node[2];
	int             dn_robin;
} tsdb_kset_t;

typedef struct tsdb_set
{
	int             kset_cnt;
	tsdb_kset_t     key_set[32];
} tsdb_set_t;

typedef enum
{
	STAT_MODE_UNKNOWN = 0,
	STAT_MODE_INCR,
	STAT_MODE_SET
} stat_mode_e;

typedef struct statistics_key
{
	char                    *name;
	unsigned long long      count;
	unsigned long long      size;
	stat_mode_e             mode;
} statistics_key_t;

typedef struct statistics
{
	char                    *host;
	int                     port;
	int                     keys_cnt;
	statistics_key_t        *keys;
} statistics_t;

typedef struct timport_key
{
	char                    *key;
	tkey_type_e             type;
	tkey_interval_e         interval;
	int                     param_cnt;
	int                     param_tm_pos;
	int                     child_cnt;
	int                     expire_time;
	int                     ignore_error;
	statistics_key_t        *stat;
	TKEY_REDIS_FILTER       redis_filter;
	TKEY_RESULT_FILTER      result_filter;
	TKEY_KEY_FILTER         key_filter;
	TKEY_HASH_FILTER        hash_filter;
	tsdb_set_t              *tsdb;
	struct timport_key      *child;
	struct timport_key      *parent;
} timport_key_t;

struct timport_cfg_file
{
	size_t          max_req_size;

	char            *log_path;
	char            *log_file;
	short           log_level;

	redis_link_t    *redis;
	int             redis_cnt;

	int             zk_disabled;
	char            *zk_servers;
	char            *zk_rnode;

	int             delay_time;
	char            *backup_path;
	char            *start_time_file;
	tsdb_set_t      tsdb;

	statistics_t    statistics;
	timport_key_t   tktree;
	int             has_ten_min;
};

struct timport_cfg_list
{
	struct supex_argv       argv_info;
	struct timport_cfg_file file_info;

	void                    (*entry_init)(void);
	void                    (*pthrd_init)(void *user);
	void                    (*reload_cfg)(void);

	void                    (*shut_down)(void);
};

void read_timport_cfg(struct timport_cfg_file *p_cfg, char *name);

