#pragma once

#include "utils.h"
#include "supex.h"
#include "sniff_task.h"

struct sniff_cfg_file
{
	short   worker_counts;
#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
	short   tasker_counts;
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

#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
	SNIFF_VMS_FCB           vmsys_init;
	SNIFF_VMS_FCB           vmsys_exit;
	SNIFF_VMS_FCB           vmsys_load;
	SNIFF_VMS_FCB           vmsys_rfsh;
	SNIFF_VMS_FCB           vmsys_call;
	SNIFF_VMS_FCB           vmsys_push;
	SNIFF_VMS_FCB           vmsys_pull;
#endif
};

