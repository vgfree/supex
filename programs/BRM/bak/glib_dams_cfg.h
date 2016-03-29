#ifndef __GLIB_DAMS_CFG_H_
#define __GLIB_DAMS_CFG_H_

#include "dams_cfg.h"

struct config_file
{
	char    program_name[MAX_NAME_SIZE];		/*应用程序名*/
	char    conf_name[MAX_PATH_SIZE];		/*配置文件名*/
};

bool glib_read_dams_cfg(int argc, char **argv, struct dams_cfg_file *volatile p_cfg, struct config_file *config);

void parse_glib_conf_file_name(int argc, char **argv, struct config_file *config);
#endif

