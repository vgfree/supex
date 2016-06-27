#pragma once

struct pole_conf
{
	char    *input_uri;
	char    *bind_uri;
	int     max_records;
	int     event_worker_counts;
	int     event_tasker_counts;
};

void config_init(struct pole_conf *p_cfg, char *name);

