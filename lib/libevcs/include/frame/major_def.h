#pragma once
#include "utils.h"
#include "evmdl.h"

#define OPT_OK                  "+OK\r\n"
#define OPT_FAILED              "-FAILED\r\n"
#define OPT_BULK_NULL           "$-1\r\n"
#define OPT_MULTI_BULK_NULL     "*0\r\n"
#define OPT_MULTI_BULK_FALSE    "*-1\r\n"
#define OPT_CMD_ERROR           "-CMD ERROR\r\n"
#define OPT_NO_THIS_CMD         "-NO THIS COMMAND\r\n"
#define OPT_KV_TOO_MUCH         "-KEY VALUE TOO MUCH\r\n"
#define OPT_NAME_TOO_LONG       "-NAME TOO LONG\r\n"
#define OPT_UNKNOW_ERROR        "-UNKNOW ERROR CODE\r\n"
#define OPT_INTERIOR_ERROR      "-INTERIOR ERROR!\r\n"
#define OPT_NO_MEMORY           "-NO MORE MEMORY!\r\n"
#define OPT_DATA_TOO_LARGE      "-DATA TOO LARGE!\r\n"
// ---> *_cfg.h
enum
{
	APPLY_FUNC_ORDER = 0,
	FETCH_FUNC_ORDER,
	MERGE_FUNC_ORDER,
	CUSTOM_FUNC_ORDER,
	// LIMIT_FUNC_ORDER,
};

enum
{
	SET_FUNC_ORDER = 0,
	DEL_FUNC_ORDER,
	MSET_FUNC_ORDER,
	HSET_FUNC_ORDER,
	HMGET_FUNC_ORDER,
	RPUSHX_FUNC_ORDER,
	LPUSHX_FUNC_ORDER,
	PUBLISH_FUNC_ORDER,
	HGETALL_FUNC_ORDER,
	GET_FUNC_ORDER,
	SADD_FUNC_ORDER,
	LRANGE_FUNC_ORDER,
	KEYS_FUNC_ORDER,
	VALUES_FUNC_ORDER,
	INFO_FUNC_ORDER,
	PING_FUNC_ORDER,
	EXISTS_FUNC_ORDER,
	SYNCSET_FUNC_ORDER,
	SYNCDEL_FUNC_ORDER,
	COMPACT_FUNC_ORDER,
	QUIT_FUNC_ORDER,
	// LIMIT_FUNC_ORDER,
};

enum
{
	UPSTREAM_FUNC_ORDER = 0,
	ONLINE_FUNC_ORDER,
	OFFLINE_FUNC_ORDER,
	// LIMIT_FUNC_ORDER,
};
#define LIMIT_FUNC_ORDER 100

// ---> *_api.h
#include "http_api/http_status.h"
#include "redis_api/redis_status.h"
#include "adopt_tasks/adopt_task.h"
/*************************************************/
#define FETCH_MAX_CNT_MSG               "-you have time travel!\r\n"
#define SERVER_BUSY_ALARM_FACTOR        10000

/* USE HTTP PROTOCOL */
struct api_list
{
	char            type;		/**< 工作模式*/
	char            *name;		/**< HTTP API URL虚地址*/
	size_t          len;		/**< HTTP API URL虚地址长度*/
	TASK_VMS_FCB    func;		/**< HTTP API 工作回调*/
};

/* USE REDIS PROTOCOL */
struct cmd_list
{
	char            type;
	TASK_VMS_FCB    func;
};

/* USE MTTP PROTOCOL */
struct mcb_list
{
	char            type;
	TASK_VMS_FCB    func;
};

bool check_finish(int ptype, int sfd);

#define MAX_LISTEN_COUNTS 10240

int socket_init(int port);

int get_redis_cmd_order(uint64_t cmd);

