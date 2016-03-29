#pragma once

#include "libmini.h"

#define MAX_LINK_INDEX 32

struct rr_link
{
	int     port;
	char    *host;
};

struct rr_cfg_file
{
	int             count;
	struct rr_link  links[MAX_LINK_INDEX];
	char            *appKey;
	char            *secret;
	char            *map_server_host;
	short           map_server_port;
};

bool read_rr_cfg(struct rr_cfg_file *p_cfg, char *name);

