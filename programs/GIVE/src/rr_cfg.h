#pragma once

#include "utils.h"

#define MAX_LINK_INDEX  32
#define MAX_CITY_SIZE   64
struct rr_link
{
	int     port;
	char    *host;
};

typedef struct sql_conf_s
{
	char            *host;
	char            *username;
	char            *password;
	char            *database;
	char            *sql;
	char            *table;
	unsigned int    port;
} sql_conf_t;

struct rr_cfg_file
{
	int             count;
	int             city_size;
	struct rr_link  links[MAX_LINK_INDEX];
	int             citycode[MAX_CITY_SIZE];
	char            *gaode_host;
	int             gaode_port;
	int             synctime;
	sql_conf_t      sqlconf;
};

bool read_rr_cfg(struct rr_cfg_file *p_cfg, char *name);

