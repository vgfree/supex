#pragma once

#include "../engine/base/utils.h"
#include <ev.h>

/* --------------                      */

#define SESSION_IO_TIMEOUT (50)			// ms

struct  session_action
{
	const char      cmd[MAX_FILE_NAME_SIZE];		// 命令名
	bool            (*action)(void *user, void *data);	// 命令回调
	const char      taskmode;				// 命令执行类型，0时为会话接口直接执行，否则使用外部分配函数执行
};

/* --------------                      */

struct session_task
{
	int                     fd;
	char                    cmd[MAX_FILE_NAME_SIZE];
	void                    *data;
	AO_SpinLockT            lock;
	struct session_action   *action;
};

/* --------------                      */

/*需要实现的接口*/

struct session_api
{
	//        void (*session_task_cb)(void*, void*);
	void (*session_dispatch_task)(struct session_task *data);
};

/* --------------                      */
extern struct session_api g_session_api;

//void session_accept_cb(struct ev_loop *loop, ev_io *ev, int revents);

bool session_response_clnt(int fd, int tm, const char *fmt, ...);

/**
 * 不可重入
 */
bool session_add_command(const struct session_action cmdset[], int cnt);

bool session_replace_command(const char cmd[], bool (*action)(void *user, void *data));

// void session_execute_action(void* user, struct session_task *service);
void session_init_dispatch(void (*session_dispatch_task)(struct session_task *));

void msmq_share_cb(struct ev_loop *loop, ev_io *w, int revents);

