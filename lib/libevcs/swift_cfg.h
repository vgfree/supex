#pragma once

#include "utils.h"
#include "supex.h"
#include "../major_def.h"

/* JSON格式配置文件 */
struct swift_cfg_file
{
	int     srv_port;
	short   worker_counts;
	short	tasker_counts;
	short   monitor_times;	/**< 监控定时器时长 默认为1S*/

	size_t  max_req_size;

	char    *log_path;
	char    *log_file;
	short   log_level;

	int     ptype;	/* http or redis */

	/* http protocol */
	short   api_counts;
	char    *api_apply;
	char    *api_fetch;
	char    *api_merge;
	char    api_names[MAX_API_COUNTS][MAX_API_NAME_LEN + 1];
};

struct swift_cfg_func
{
	char            type;
	TASK_VMS_FCB    func;
};

struct swift_cfg_list
{
	struct supex_argv       argv_info;
	struct swift_cfg_file   file_info;
	struct swift_cfg_func   func_info[LIMIT_FUNC_ORDER];

	void                    (*entry_init)(void);
	void                    (*pthrd_init)(void *user);
	void                    (*reload_cfg)(void);

	void                    (*shut_down)(void);
	
	bool                    (*task_lookup)(void *user, void *task);	/**< 弹出任务回调*/
	bool                    (*task_report)(void *user, void *task);	/**< 推入任务回调*/

	TASK_VMS_FCB		vmsys_init;
	TASK_VMS_FCB		vmsys_exit;
	TASK_VMS_FCB		vmsys_cntl;
	TASK_VMS_FCB		vmsys_rfsh;
	TASK_VMS_FCB		vmsys_idle;
	TASK_VMS_FCB		vmsys_monitor;
};

