#pragma once

struct pole_conf
{
	char    *input_uri;
	char    *bind_uri;
	int     max_records;
	int     thread_number;	// 如果宏编译TC_COROUTINE 则thread_number生效;
};

void config_init(struct pole_conf *p_cfg, char *name);

