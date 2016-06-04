#pragma once

#include "base/utils.h"

#define MAX_APP_COUNTS 100

struct child_cfg_list
{
	short           app_counts;
	char            app_names[MAX_APP_COUNTS][MAX_FILE_NAME_SIZE];
	char            app_msmqs[MAX_APP_COUNTS][MAX_FILE_NAME_SIZE];
	char            app_redis_host[MAX_APP_COUNTS][MAX_FILE_NAME_SIZE];
	unsigned short  app_redis_port[MAX_APP_COUNTS];
};

void load_child_cfg_file(struct child_cfg_list *p_cfg, char *name);

