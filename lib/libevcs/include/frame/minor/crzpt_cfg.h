#pragma once

#include "utils.h"
#include "engine/supex.h"
#include "engine/adopt_tasks/adopt_task.h"

struct crzpt_cfg_file
{
	short   worker_counts;
#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
	short   tasker_counts;
#endif

	char    *log_path;
	char    *log_file;
	short   log_level;
};

struct crzpt_cfg_list
{
	struct supex_argv       argv_info;
	struct crzpt_cfg_file   file_info;

	void                    (*entry_init)(void);

	bool                    (*task_lookup)(void *user, void *task);
	bool                    (*task_report)(void *user, void *task);

#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
	TASK_VMS_FCB            vmsys_init;
	TASK_VMS_FCB            vmsys_exit;
	TASK_VMS_FCB            vmsys_load;
	TASK_VMS_FCB            vmsys_rfsh;
	TASK_VMS_FCB            vmsys_call;
	TASK_VMS_FCB            vmsys_push;
	TASK_VMS_FCB            vmsys_pull;
#endif
	void                    (*store_firing)(const char *name, size_t block_size, size_t wb_size, size_t lru_size, short bloom_size);
	int                     (*store_insert)(const char *key, size_t klen, const char *value, size_t vlen);
};

