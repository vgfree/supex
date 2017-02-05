#pragma once

#include "async_ops.h"
#include "async_prt.h"
#include "../cache/cache.h"

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
	void    *loop;

	/* Hooks that are called when the library expects to start
	 * reading/writing. These functions should be idempotent. */
	void    (*add_recv)(void **ev_impl);
	void    (*del_recv)(void **ev_impl);
	void    (*add_send)(void **ev_impl);
	void    (*del_send)(void **ev_impl);
	void    (*produce)(void **ev_impl);
	void    (*cleanup)(void **ev_impl);
};

struct async_obj;
struct command_node;

typedef void (*ASYNC_CALL_BACK)(struct async_obj *obj, struct command_node *cmd, void *data);

struct command_node
{
	int                     sfd;
	bool                    ok;
	enum
	{
		ASYNC_OK = 0,
		ASYNC_ER_SOCKET,
		ASYNC_ER_PARSE,
		ASYNC_ER_TIMEOUT,
	}                       err;

	/*proto details*/
	char	fops[3];
	short	step;
	short	cycle;
	char *r_data;
	size_t r_size;
	char *w_data;
	size_t w_size;
	/*cmd dispose*/
	struct async_ops	*driver;
	/*cmd parser*/
	struct async_prt 	parser;

	void                    *ev_impl;
	struct ev_settings      *ev_hook;

	/*cmd callback*/
	ASYNC_CALL_BACK         fcb;
	void                    *usr;
	bool                    hit;

	/*cmd data*/
	struct cache            r_cache;
	struct cache            w_cache;
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
typedef void (__LINK_CALL_BACK)(const struct async_obj *, void *data);
typedef void (__RECY_CALL_BACK)(const struct async_obj *, void *data);

/* Context for an async connection to Redis */
struct async_obj
{
	__LINK_CALL_BACK        *__midway_stop;
	__RECY_CALL_BACK        *__finish_work;
	void                    *data;

	/* Event library data and hooks */
	struct ev_settings      settings;

	/* Regular command callbacks */
	char                    qtype;
	char                    ntype;
	struct command_list     clist;
};

void async_obj_initial(struct async_obj *obj, int peak, enum queue_type qtype, enum nexus_type ntype, struct ev_settings *settings);

struct command_node     *async_obj_command(struct async_obj *obj, enum proto_type ptype, int sfd,
	const char *fops, const char *data1, size_t size1, const char *data2, size_t size2,
	ASYNC_CALL_BACK fcb, void *usr);

void async_obj_startup(struct async_obj *obj);

void async_obj_suspend(struct async_obj *obj);

int async_obj_distory(struct async_obj *obj);

/*获取当前工作任务*/
struct command_node     *async_obj_current(struct async_obj *obj);

/* Handle read/write events */
void _async_obj_handle_send(struct async_obj *obj, struct command_node *cmd);

void _async_obj_handle_recv(struct async_obj *obj, struct command_node *cmd);

