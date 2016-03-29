#pragma once

#include "utils.h"
#include "supex.h"

struct sniff_cfg_file
{
	short   worker_counts;
#ifdef OPEN_SCCO
	short   sharer_counts;
#endif

	char    *log_path;
	char    *log_file;
	short   log_level;
};

struct sniff_cfg_list
{
	struct supex_argv       argv_info;
	struct sniff_cfg_file   file_info;

	void                    (*entry_init)(void);

	bool                    (*task_lookup)(void *user, void *task);
	bool                    (*task_report)(void *user, void *task);

#ifdef OPEN_SCCO
	int                     (*vmsys_init)(void *user, void *task, int step);
	int                     (*vmsys_exit)(void *user, void *task, int step);
	int                     (*vmsys_load)(void *user, void *task, int step);
	int                     (*vmsys_rfsh)(void *user, void *task, int step);
	int                     (*vmsys_call)(void *user, void *task, int step);
	int                     (*vmsys_push)(void *user, void *task, int step);
	int                     (*vmsys_pull)(void *user, void *task, int step);
#else
	int                     (*vmsys_init)(void *user, void *task);
	int                     (*vmsys_exit)(void *user, void *task);
	int                     (*vmsys_load)(void *user, void *task);
	int                     (*vmsys_rfsh)(void *user, void *task);
	int                     (*vmsys_call)(void *user, void *task);
	int                     (*vmsys_push)(void *user, void *task);
	int                     (*vmsys_pull)(void *user, void *task);
#endif
};

