#pragma once
#include "utils.h"
#include "def_task.h"

/* choose use http or redis */
enum major_proto_type
{
	USE_HTTP_PROTO = 0,
#ifdef _mttptest
	USE_REDIS_PROTO,
	USE_MTTP_PROTO
#else
	USE_REDIS_PROTO
#endif
};

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
	HMGET_FUNC_ORDER,
	HSET_FUNC_ORDER,
	RPUSHX_FUNC_ORDER,
	LPUSHX_FUNC_ORDER,
	PUBLISH_FUNC_ORDER,
	HGETALL_FUNC_ORDER,
	DEL_FUNC_ORDER,
	MSET_FUNC_ORDER,
	GET_FUNC_ORDER,
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
#define LIMIT_FUNC_ORDER 100

// ---> *_api.h
#include "http.h"

#ifdef _mttptest
  #include "mttp.h"
#endif

#include "redis_parse.h"
/*************************************************/
#define FETCH_MAX_CNT_MSG               "-you have time travel!\r\n"
#define SERVER_BUSY_ALARM_FACTOR        10000
/*************************************************/
#define X_DATA_NO_ALL                   2
#define X_DATA_IS_ALL                   1

#define X_DONE_OK                       0
#define X_IO_ERROR                      -1
#define X_DATA_TOO_LARGE                -2
#define X_MALLOC_FAILED                 -3
#define X_PARSE_ERROR                   -4
#define X_INTERIOR_ERROR                -5
#define X_REQUEST_ERROR                 -6
#define X_EXECUTE_ERROR                 -7
#define X_REQUEST_QUIT                  -8
#define X_KV_TOO_MUCH                   -9
#define X_NAME_TOO_LONG                 -10
/*************************************************/

/* USE HTTP PROTOCOL */
struct api_list
{
	char            type;		/**< 工作模式*/
	char            *name;		/**< HTTP API URL虚地址*/
	size_t          len;		/**< HTTP API URL虚地址长度*/
	TASK_CALLBACK   func;		/**< HTTP API 工作回调*/
};

/* USE REDIS PROTOCOL */
struct cmd_list
{
	char            type;
	TASK_CALLBACK   func;
};

#ifdef _mttptest
struct mttp_list
{
	char            type;
	TASK_CALLBACK   func;
};
#endif

#include <ev.h>
#include <arpa/inet.h>

#include "list.h"
#include "net_cache.h"

enum
{
	NOIN_LOOP = 0,
	INEV_LOOP,
	WORK_LOOP
};

struct data_node
{
	/*whenever can't clean when reset data_node*/
	/***base attribute***/
	int                     sfd;		/**< 关联的描述符*/

	/*should clean when reset data_node*/
	/***ev***/
	ev_io                   io_watcher;	/**< IO watcher*/
	ev_timer                timer_watcher;	/**< 超时 watcher*/
	/***R***/
	struct net_cache        recv;		/**< 接收缓存*/

	/***W***/
	struct net_cache        send;		/**< 发送缓存*/

	/****S***/
#ifdef OPEN_TIME_OUT
	int                     at_step;
	bool                    is_over;
#endif
	int                     control;	/*cmd dispose*/
	int                     ptype;		/* http or redis */
	union
	{
		/* USE HTTP PROTOCOL */
		struct http_parse_info  http_info;
		/* USE REDIS PROTOCOL */
		struct redis_parse_info redis_info;
#ifdef _mttptest
		struct mttp_parse_info  mttp_info;
#endif
	};
};

/**地址结点*/
struct addr_node
{
	enum
	{
		NO_WORKING = 0,
		IS_WORKING = 1,
	}                       work_status;				/*io dispose*/
	struct data_node        *addr;					/**< 关联的数据节点*/
	struct queue_item       recv_item;
	struct queue_item       send_item;
	int                     port;					/**< 客户端本端端口*/
	char                    szAddr[INET_ADDRSTRLEN];		/**< 客户端远端地址 */
};

void pools_init(int proto_type);

struct addr_node        *mapping_addr_node(int fd);

struct data_node        *get_pool_addr(int fd);

void del_pool_addr(int fd);

void get_cache_data(int sfd, void *buff, int *size);

#define MAX_LISTEN_COUNTS 10240

int socket_init(int port);

int app_lua_get_head_data(lua_State *L);

int app_lua_get_body_data(lua_State *L);

int app_lua_get_path_data(lua_State *L);

int app_lua_get_uri_args(lua_State *L);

int app_lua_add_send_data(lua_State *L);

int app_lua_get_recv_buf(lua_State *L);

