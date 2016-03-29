#pragma once

#include "common.h"

struct follow_node
{
	char                    *name;
	struct follow_node      *next;
};

struct loghit_cfg_argv
{
	char    conf_name[MAX_FILE_NAME_SIZE];
	char    serv_name[MAX_FILE_NAME_SIZE];
};

struct loghit_cfg_file
{
	short                   port;
	char                    *host;

	char                    *unique_file;
	char                    *loghitDB_path;
	int                     space_usleep;
	struct follow_node      *list;
};

struct loghit_cfg_list
{
	struct loghit_cfg_argv  argv_info;
	struct loghit_cfg_file  file_info;
};

void load_loghit_cfg_argv(struct loghit_cfg_argv *p_cfg, int argc, char **argv);

void load_loghit_cfg_file(struct loghit_cfg_file *p_cfg, char *name);

