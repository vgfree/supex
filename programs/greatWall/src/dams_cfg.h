#pragma once

#include "utils.h"

#define MAX_LINK_INDEX  32
#define NO_SET_UP       '0'
#define IS_SET_UP       '1'

struct dams_link
{
	int     port;
	char    *host;
};

struct dams_cfg_file
{
	int                     per_peak_cnt_count;
	int                     count;
	char                    fresh[MAX_LINK_INDEX];
	char                    delay[MAX_LINK_INDEX];
	struct dams_link        links[MAX_LINK_INDEX];
};

bool read_dams_cfg(struct dams_cfg_file *p_cfg, char *name);

