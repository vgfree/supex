#pragma once

#include "x_utils.h"

struct mfptp_cfg_argv
{
	char    conf_name[MAX_FILE_NAME_SIZE];
	char    serv_name[MAX_FILE_NAME_SIZE];
	char    msmq_name[MAX_FILE_NAME_SIZE];
};

struct mfptp_cfg_file
{
	short   srv_port;
	short   front_port;
	short   back_port;
	short   usr_msg_port;
	short   edit_user_info_port;
	short   gp_msg_port;
	short   worker_counts;
	char    *redis_address;
	short   redis_port;
	short   user_online_status;

	size_t  max_req_size;

	char    *log_path;
	char    *log_file;
	short   log_level;
};

struct mfptp_cfg_list
{
	struct mfptp_cfg_argv   argv_info;
	struct mfptp_cfg_file   file_info;

	void                    (*entry_init)(void);
	void                    (*pthrd_init)(void *user);
	int                     (*vmsys_init)(void *W);

	bool                    (*task_lookup)(void *user, void *task);
	bool                    (*task_report)(void *user, void *task);

	int                     (*drift_away)(void *W);
	int                     (*drift_come)(void *W);
};

