#pragma once
#include <ev.h>
#include "base/qlist.h"
#include "cache/cache.h"
#include "async_comm.h"

struct pipe_module
{
	int             pfds[2];
	struct ev_io    pipe_watcher;		/**< 读写watcher accept watcher for new connect */
	void            (*usr_pipe_fcb)(struct pipe_module *);
	int             sfd;
	void            *data;
};

void evmdl_pipe_init(struct pipe_module *p_mdl, void (*usr_pipe_fcb)(struct pipe_module *), void *data);

void evmdl_pipe_call(struct ev_loop *loop, struct pipe_module *p_mdl);

void evmdl_pipe_stop(struct ev_loop *loop, struct pipe_module *p_mdl);

struct list_module
{
	struct queue_list       list;		/**< queue of new request to handle */
	struct ev_async         async_watcher;	/**< 异步watcher async watcher for new connect */
	void                    (*usr_list_fcb)(struct list_module *);
	int                     sfd;
	void                    *data;
};

void evmdl_list_init(struct list_module *p_mdl, void (*usr_list_fcb)(struct list_module *), void *data);

void evmdl_list_call(struct ev_loop *loop, struct list_module *p_mdl);

void evmdl_list_stop(struct ev_loop *loop, struct list_module *p_mdl);

struct accept_module
{
	int             listenfd;
	struct ev_io    accept_watcher;			/* accept watcher for new connect */
	void            (*usr_accept_fcb)(struct accept_module *);
	int             sfd;
	void            *data;
};

void evmdl_accept_init(struct accept_module *p_mdl, void (*usr_accept_fcb)(struct accept_module *), void *data, int listenfd);

void evmdl_accept_call(struct ev_loop *loop, struct accept_module *p_mdl);

void evmdl_accept_stop(struct ev_loop *loop, struct accept_module *p_mdl);

struct update_module
{
	struct ev_timer update_watcher;
	void            (*usr_update_fcb)(struct update_module *);
	void            *data;
};

void evmdl_update_init(struct update_module *p_mdl, void (*usr_update_fcb)(struct update_module *), void *data);

void evmdl_update_call(struct ev_loop *loop, struct update_module *p_mdl);

void evmdl_update_stop(struct ev_loop *loop, struct update_module *p_mdl);

struct reload_module
{
	struct ev_stat  file_watcher;
	void            (*usr_reload_fcb)(struct reload_module *);
	void            *data;
};

void evmdl_reload_init(struct reload_module *p_mdl, void (*usr_reload_fcb)(struct reload_module *), void *data, char *file);

void evmdl_reload_call(struct ev_loop *loop, struct reload_module *p_mdl);

void evmdl_reload_stop(struct ev_loop *loop, struct reload_module *p_mdl);

struct signal_module
{
	struct ev_signal        signal_watcher;
	void                    (*usr_signal_fcb)(struct signal_module *);
	void                    *data;
};

void evmdl_signal_init(struct signal_module *p_mdl, void (*usr_signal_fcb)(struct signal_module *), void *data, int signo);

void evmdl_signal_call(struct ev_loop *loop, struct signal_module *p_mdl);

void evmdl_signal_stop(struct ev_loop *loop, struct signal_module *p_mdl);

struct monitor_module
{
	double                  tstamp;
	struct ev_periodic      monitor_watcher;	// 监控事件
	void                    (*usr_monitor_fcb)(struct monitor_module *);
	void                    *data;
};

void evmdl_monitor_init(struct monitor_module *p_mdl, void (*usr_monitor_fcb)(struct monitor_module *), void *data, double tstamp);

void evmdl_monitor_call(struct ev_loop *loop, struct monitor_module *p_mdl);

void evmdl_monitor_stop(struct ev_loop *loop, struct monitor_module *p_mdl);

/**
 * 分段传输适合upstream/dwstream，不适合asking<-->answer
 * 分段传输只适合swift模式,通过pipetask在smart上实现无意义。因为smart上只有asking<-->answer，http/redis都不适用分段传输,无用武之地
 * 分段传输时(有数据start io/无stop io)
 */
struct irecv_module
{
	ev_io           io_watcher;		/**< IO watcher*/
	struct cache    cache;			/**< 接收缓存*/
	int             ptype;			/* http or redis */
	PROTO_CALL_BACK *proto_handler_work;
	PROTO_CALL_BACK *proto_handler_init;
	PROTO_CALL_BACK *proto_handler_free;
	union
	{
		/* USE HTTP PROTOCOL */
		struct http_parse_info  http_info;
		/* USE REDIS PROTOCOL */
		struct redis_parse_info redis_info;
	}               parse;
	enum
	{
		IRECV_STATUS_ISOK = 0,
		IRECV_STATUS_FAIL,
		IRECV_STATUS_OVER
	}               status;
	void            (*usr_irecv_fcb)(struct irecv_module *);
	int             sfd;
	void            *data;
};

void evmdl_irecv_init(struct irecv_module *p_mdl, void (*usr_irecv_fcb)(struct irecv_module *), void *data, int sfd, int ptype);

void evmdl_irecv_call(struct ev_loop *loop, struct irecv_module *p_mdl);

void evmdl_irecv_stop(struct ev_loop *loop, struct irecv_module *p_mdl);

struct osend_module
{
	ev_io           io_watcher;		/**< IO watcher*/
	struct cache    cache;			/**< 接收缓存*/
	bool            finish;
	enum
	{
		OSEND_STATUS_ISOK = 0,
		OSEND_STATUS_OVER
	}               status;
	void            (*usr_osend_fcb)(struct osend_module *);
	int             sfd;
	void            *data;
};

void evmdl_osend_init(struct osend_module *p_mdl, void (*usr_osend_fcb)(struct osend_module *), void *data, int sfd, bool finish);

void evmdl_osend_call(struct ev_loop *loop, struct osend_module *p_mdl);

void evmdl_osend_stop(struct ev_loop *loop, struct osend_module *p_mdl);

struct iotime_module
{
	enum
	{
		NOIN_LOOP = 0,
		INEV_LOOP,
		WORK_LOOP
	}                       at_step;
	bool                    is_over;
	ev_timer                timer_watcher;	/**< 超时 watcher*/
	struct osend_module     *send;
	struct irecv_module     *recv;
	void                    (*usr_iotime_fcb)(struct iotime_module *);
	int                     sfd;
	void                    *data;
};

void evmdl_iotime_init(struct iotime_module *p_mdl, void (*usr_iotime_fcb)(struct iotime_module *), void *data, int sfd, struct irecv_module *recv, struct osend_module *send, double overtime);

void evmdl_iotime_call(struct ev_loop *loop, struct iotime_module *p_mdl);

void evmdl_iotime_stop(struct ev_loop *loop, struct iotime_module *p_mdl);

struct upstream_module
{
	ev_io           io_watcher;		/**< IO watcher*/
	struct cache    cache;			/**< 接收缓存*/
	int             ptype;			/* http or redis */
	PROTO_CALL_BACK *proto_handler_work;
	PROTO_CALL_BACK *proto_handler_init;
	PROTO_CALL_BACK *proto_handler_free;
	union
	{
		struct http_parse_info  http_info;
		struct redis_parse_info redis_info;
		struct mttp_parse_info  mttp_info;
	}               parse;
	enum
	{
		UPSTREAM_STATUS_ISOK = 0,
		UPSTREAM_STATUS_OVER
	}               status;
	void            (*usr_upstream_fcb)(struct upstream_module *);
	int             sfd;
	void            *data;
	char            *package_data;
	long            package_size;
};

void evmdl_upstream_init(struct upstream_module *p_mdl, void (*usr_upstream_fcb)(struct upstream_module *), void *data, int sfd, int ptype);

void evmdl_upstream_call(struct ev_loop *loop, struct upstream_module *p_mdl);

void evmdl_upstream_stop(struct ev_loop *loop, struct upstream_module *p_mdl);

struct dwstream_module
{
	ev_io           io_watcher;		/**< IO watcher*/
	struct cache    cache;			/**< 接收缓存*/
	enum
	{
		DWSTREAM_STATUS_ISOK = 0,
		DWSTREAM_STATUS_OVER
	}               status;
	void            (*usr_dwstream_fcb)(struct dwstream_module *);
	int             sfd;
	void            *data;
};

void evmdl_dwstream_init(struct dwstream_module *p_mdl, void (*usr_dwstream_fcb)(struct dwstream_module *), void *data, int sfd);

void evmdl_dwstream_call(struct ev_loop *loop, struct dwstream_module *p_mdl);

void evmdl_dwstream_stop(struct ev_loop *loop, struct dwstream_module *p_mdl);

/* choose use http or redis */
enum major_proto_type
{
	USE_HTTP_PROTO = 0,
	USE_REDIS_PROTO,
	USE_MTTP_PROTO
};

struct data_node
{
	/****************************************************************/
	/***R***/
	struct irecv_module     mdl_recv;
	/***W***/
	struct osend_module     mdl_send;
	/***S***/
#ifdef OPEN_TIME_OUT
	struct iotime_module    mdl_iotime;
	int			at_step;
#endif
	int                     control;	/*cmd dispose*/

	/****************************************************************/
	/***R***/
	struct upstream_module  mdl_up;
	/***W***/
	struct dwstream_module  mdl_dw;
	/****************************************************************/
};

/**地址结点*/
struct pool_node
{
	struct data_node        *addr;					/**< 关联的数据节点*/
	uint64_t                cid;
	int                     port;					/**< 客户端本端端口*/
	char                    szAddr[INET_ADDRSTRLEN];		/**< 客户端远端地址 */
};

void pools_init(int proto_type);

struct pool_node        *mapping_pool_addr(int fd);

struct data_node        *get_pool_data(int fd);

void del_pool_data(int fd);

/*获取非0的无符号整数*/
uint64_t make_a_cid(void);

