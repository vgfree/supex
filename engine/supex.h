#pragma once

#include "ev.h"
#include "utils.h"
#include "scco.h"
#include "list.h"
#include "evcoro_scheduler.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <sys/socket.h>
#include <netinet/in.h>

/* ------------                            */

struct supex_scco
{
	unsigned int            num;
	unsigned int            tsz;
	unsigned int            nxt;
	bool                    have;
	void                    *data;
	bool                    (*safe_filter)(void *data, void *addr);
	bool                    (*task_lookup)(void *data, void *addr);
	void                    *(*task_supply)(struct schedule *S, void *self);
	void                    *(*task_handle)(struct schedule *S, void *step);
	void                    *temp;	/*No.(num + 1)*/
	void                    *task;	/*num + 1*/
	union virtual_system    *VMS;
	struct coroutine        *C;
	struct schedule         S;
};

void supex_scco_loop(struct supex_scco *scco);

/* ------------                            */

/**
 * 任务线性处理模型
 */
struct supex_line
{
	unsigned int            tsz;					/**< 任务大小*/
	void                    *task;					/**< 取出任务的副本*/
	void                    *data;					/**< 所属线程的线程句柄*/

	union virtual_system    VMS;					/**< 所属线程的LUA虚拟机*/
	bool                    (*task_lookup)(void *data, void *addr);	/**< 取任务回调 supex_task_pull()*/
	void                    (*task_handle)(void *data, void *addr);	/**< 处理任务回调函数 smart_task_handle()*/
};

void supex_line_loop(struct supex_line *line);

/* ------------                            */

struct supex_evuv
{
	unsigned int            tsz;
	void                    *task;
	void                    *data;

	union virtual_system    VMS;
	bool                    (*task_lookup)(void *data, void *addr);
	void                    (*task_handle)(void *data, void *addr);

	unsigned int            depth;
	struct ev_loop          *loop;
	struct ev_async         async_watcher;
	struct ev_idle          idle_watcher;
#if 0
	struct ev_periodic      periodic_watcher;
	struct ev_check         check_watcher;
#endif
};

void supex_evuv_loop(struct supex_evuv *evuv);

void supex_evuv_wake(struct supex_evuv *evuv);

/* ------------                            */
/* evcoro 模式 */
struct supex_evcoro
{
	unsigned int            num;
	unsigned int            tsz;
	void                    *task;
	void                    *data;
	void                    *temp;

	union virtual_system    *VMS;
	bool                    (*task_lookup)(void *data, void *addr);
	void                    (*task_handle)(void *data, void *addr);

	struct evcoro_scheduler *scheduler;	/** 协程集群句柄 */
};

struct evcoro_task
{
	unsigned int            idx;
	struct supex_evcoro     *evcoro;
};

void supex_evcoro_loop(struct supex_evcoro *evcoro);

/* ------------                            */
struct supex_task_base
{
	unsigned int shift;
};

struct supex_task_node
{
	unsigned int size;
	union
	{
		struct supex_task_base  *base;
		void                    *data;
	};
};
/** 任务队列*/
struct supex_task_list
{
	unsigned int    max;	/**< 最大容量*/
	unsigned int    all;	/**< 最大空间*/
	unsigned int    tsz;	/**< 每个元素的大小*/
	unsigned int    isz;	/**< 累计压入*/
	unsigned int    osz;	/**< 累计弹出*/
	AO_T            tasks;	/**< 任务数量*/
	void            *slots;	/**< 队列BUFFER*/

	unsigned int    head;	/**< 当前队列头槽位号*/
	unsigned int    tail;	/**< 当前队列尾槽位号*/

	AO_SpinLockT    w_lock;	/**< 压入锁*/
	AO_SpinLockT    r_lock;	/**< 弹出锁*/
};

void supex_node_init(struct supex_task_node *node, void *data, unsigned int size);

void supex_node_copy(struct supex_task_node *dst, struct supex_task_node *src);

/**
 * 初始化
 * @param list 被初始化队列
 * @param tsz  元素大小
 * @param max  最大容量
 */
void supex_task_init(struct supex_task_list *list, unsigned int tsz, unsigned int max);

/**
 * 将元素的副本压入队列尾部
 * @param list 操作队列
 * @param task 被压入队列的元素
 * @return 是否推入成功true/false
 */
bool supex_task_push(struct supex_task_list *list, void *task);

/**
 * 从队列的头部弹出一个元素的副本
 * @param list 操作队列
 * @param task 被弹出队列的元素
 * @return 是否获取成功true/false
 */
bool supex_task_pull(struct supex_task_list *list, void *task);

/**
 * 查看队列头数据
 */
bool supex_task_top(struct supex_task_list *list, void *task);

/**
 * 遍历队列
 */
void supex_task_travel(struct supex_task_list *list, bool (*travel)(void *, size_t size, void *), void *data);

/* ------------                            */

/*
 * 命令行参数
 */
/** 命令行参数*/
struct supex_argv
{
	char    conf_name[MAX_FILE_PATH_SIZE];	/**< 配置文件名，有命令行参数-c传递，缺省值为<pro>_conf.json*/
	char    serv_name[MAX_FILE_NAME_SIZE];	/**< 应用程序名*/
	char    msmq_name[MAX_FILE_PATH_SIZE];	/**< POSIX消息队列名，默认为应用程序全路径*/
};

bool load_supex_args(struct supex_argv *p_cfg, int argc, char **argv,
	const char *parseopt, bool (*optparser)(int, void *),
	void *user);

/* ------------                            */

/*
 * 网络包统计
 */

struct netpkg_stat
{
	struct sockaddr_storage ip;		/**< ip信息*/
	int                     sfd;		/**< 当前被计数的描述符*/
	AO_T                    counter;	/**< 计数器*/
	struct timeval          firstconn;	/**< 首次统计时间*/
	struct timeval          newstat;	/**< 最近一次统计时间*/
};

extern SListT g_netpkg_stat;

/**
 * 初始化
 */
bool netpkg_init();

/**
 * 增加一个统计节点
 * @param saddr 统计节点的IP信息，如果为NULL，则由对应的描述符获取IP信息
 * @param sockfd统计节点的描述符
 */
void netpkg_addconnect(SListT list, struct sockaddr *saddr, socklen_t slen, int sockfd);

/**
 * 使用描述符增加一次统计
 */
void netpkg_increase(SListT list, int sockfd);

/**
 * 断开描述符对应的节点的统计
 */
void netpkg_delconnect(SListT list, int sockfd);

/**
 * 打印统计节点信息
 * @param desfd 如果大于-1则输出到此描述符
 * @param filepath 如果desfd 小于 0 则打开次路径的文件，并输出信息到其中
 */
void netpkg_printstat(SListT list, int desfd, const char *filepath);

/* ------------                            */

/*
 * libkv句柄池
 */
struct kvpool_handle
{
	char    name[64];
	void    *hdl;
};

extern SListT g_kvhandle_pool;

/**
 * 初始化
 */
bool kvpool_init();

/**
 * 在lua层以句柄名称调用，查找或创建对应的句柄，并返回该句柄的libkv句柄的C指针
 */
int search_kvhandle(lua_State *L);

/**
 * 在C层已句柄名称调用，查找或创建对应的句柄，并返回该句柄
 */
struct kvpool_handle    *kvpool_search(SListT list, const char *name);

/* ------------                            */

