#pragma once

#include "utils.h"

// leveldb 参数设置
#define HIT_LDB_NAME            "loghitDB"
#define HIT_LDB_BLK_SIZE        (32 * 1024)
#define HIT_LDB_WB_SIZE         (64 * 1024 * 1024)
#define HIT_LDB_LRU_SIZE        (64 * 1024 * 1024)
#define HIT_LDB_BLOOM_SIZE      (10)

struct loghub_cfg_argv
{
	char    conf_name[MAX_FILE_NAME_SIZE];
	char    serv_name[MAX_FILE_NAME_SIZE];
};

struct loghub_cfg_file
{
	short port;
};

struct loghub_cfg_list
{
	struct loghub_cfg_argv  argv_info;
	struct loghub_cfg_file  file_info;
};

void load_loghub_cfg_argv(struct loghub_cfg_argv *p_cfg, int argc, char **argv);

void load_loghub_cfg_file(struct loghub_cfg_file *p_cfg, char *name);

