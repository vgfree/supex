#pragma once

#include "utils.h"
#include "engine/supex.h"
#include "../major_def.h"

/** JSON格式配置文件*/
struct smart_cfg_file
{
	int     srv_port;	/**< 端口号*/
	short   hander_counts;	/**< 客户端读写线程数*/
	short   worker_counts;	/**< LUA工作线程数*/
	short   monitor_times;	/**< 监控定时器时长 默认为1S*/
#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
	short   tasker_counts;
#endif
	size_t  max_req_size;	/**< 读写缓存最大值*/

	char    *log_path;	/**< 日志路径*/
	char    *log_file;	/**< 日志文件名*/
	short   log_level;	/**< 输出日志级别*/

	int     ptype;		/* 协议判断，默认为http协议*/

	/* USE_HTTP_PROTOCOL */
	short   api_counts;						/**< 支持的HTTP API数量*/
	char    *api_apply;						/**< HTTP API的URL 业务数据处理*/
	char    *api_fetch;						/**< HTTP API的URL 查询LUA的模块*/
	char    *api_merge;						/**< HTTP API的URL 修改lua的模块*/
	char    api_names[MAX_API_COUNTS][MAX_API_NAME_LEN + 1];	/**< HTTP API的URL 自定义*/
};

struct smart_cfg_func
{
	char            type;
	TASK_VMS_FCB    func;
};

struct smart_cfg_list
{
	struct supex_argv       argv_info;				/**< 配置文件*/
	struct smart_cfg_file   file_info;				/**< 配置文件*/
	struct smart_cfg_func   func_info[LIMIT_FUNC_ORDER];		/**< HTTP API 回调函数数组*/

	void                    (*entry_init)(void);			/**< 初始化回调*/
	void                    (*reload_cfg)(void);			/**< 重载配置文件回调*/

	bool                    (*task_lookup)(void *user, void *task);	/**< 弹出任务回调*/
	bool                    (*task_report)(void *user, void *task);	/**< 推入任务回调*/

	TASK_VMS_FCB            vmsys_init;				/**< 各个工作线程LUA虚拟机初始化回调*/
	TASK_VMS_FCB            vmsys_exit;				/**< LUA虚拟机销毁回调*/
	TASK_VMS_FCB            vmsys_cntl;				/**< */
	TASK_VMS_FCB            vmsys_rfsh;
	TASK_VMS_FCB            vmsys_monitor;
};

