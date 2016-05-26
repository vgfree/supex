#pragma once

#include "../async_comm.h"
#include "../cache/cache.h"

#define ASYNC_OK        0
#define ASYNC_ERR       -1

enum proto_type
{
	PROTO_TYPE_HTTP = 0,
	PROTO_TYPE_REDIS
};
enum queue_type
{
	QUEUE_TYPE_FIFO = 0,
	QUEUE_TYPE_CORO
};
enum nexus_type
{
	NEXUS_TYPE_TEAM = 0,
	NEXUS_TYPE_SOLO
};

struct ev_settings
{
	// void *data;
	void    *loop;

	/* Hooks that are called when the library expects to start
	 * reading/writing. These functions should be idempotent. */
	void    (*add_recv)(void *privdata);
	void    (*del_recv)(void *privdata);
	void    (*add_send)(void *privdata);
	void    (*del_send)(void *privdata);
	void    (*cleanup)(void *privdata);
	void    (*produce)(void *privdata);
};

struct async_obj;

typedef void (*ASYNC_CALL_BACK)(struct async_obj *obj, void *reply, void *data);

struct command_node
{
	int                     sfd;
	bool                    done;	// NEW

	void                    *ev_work;
	struct ev_settings      *ev_hook;
	/*cmd dispose*/
	char                    ptype;
	PROTO_CALL_BACK         *proto_handler_work;
	PROTO_CALL_BACK         *proto_handler_init;
	/*cmd parser*/
	int                     control;
	union
	{
		struct http_parse_info  http_info;
		struct redis_parse_info redis_info;
	}                       parse;

	/*cmd callback*/
	void                    *privdata;
	ASYNC_CALL_BACK         fcb;

	/*cmd data*/
	struct cache            cache;
	struct command_node     *next;	/* simple singly linked list */
};
struct command_list
{
	int                     peak;
	int                     have;
	int                     done;
	struct command_node     base;
	struct command_node     *head;
	struct command_node     *tail;
	struct command_node     *work;
	struct command_node     *hold;
};

/* Connection callback prototypes */
typedef void (LINK_CALL_BACK)(const struct async_obj *, void *data);
typedef void (RECY_CALL_BACK)(const struct async_obj *, void *data);

/* Context for an async connection to Redis */
struct async_obj
{
	/* Event library data and hooks */
	struct ev_settings      settings;
	int                     efd;

	void                    *data;
	LINK_CALL_BACK          *sys_midway_stop;
	RECY_CALL_BACK          *sys_finish_work;
	LINK_CALL_BACK          *usr_midway_stop;
	RECY_CALL_BACK          *usr_finish_work;

	/* Regular command callbacks */
	char                    qtype;
	char                    ntype;
	struct command_list     replies;
};

void async_obj_init_list(struct async_obj *obj, enum queue_type qtype, enum nexus_type ntype, int peak);

void async_obj_bind_hook(struct async_obj *obj, struct ev_settings *settings);

struct command_node     *async_obj_append_cmd(struct async_obj *obj, enum proto_type ptype, int sfd, ASYNC_CALL_BACK fcb, void *privdata, const char *data, size_t size);

void async_obj_begin(struct async_obj *obj);

int async_obj_free(struct async_obj *obj);

// 添加获取当前工作任务函数//TODO

/* Handle read/write events */
void _async_obj_handle_send(struct async_obj *obj, struct command_node *cmd);

void _async_obj_handle_recv(struct async_obj *obj, struct command_node *cmd);

