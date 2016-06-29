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

typedef struct redis_link
{
	char    *host;
	int     port;
	int     idx;
} redis_link_t;

typedef enum
{
	STAT_MODE_UNKNOWN = 0,
	STAT_MODE_INCR,
	STAT_MODE_SET
} stat_mode_e;

struct timport_cfg_file
{
	redis_link_t    *redis;
	int             redis_cnt;
	int             delay_time;
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

