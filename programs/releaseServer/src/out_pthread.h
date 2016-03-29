#pragma once
#include <ev.h>

typedef struct
{
	/*框架层*/
	struct ev_loop  *loop;
	void            *cq;						/*线程关联的队列*/
	ev_async        async_watcher;					/*外部在队列添加数据后，向该线程发送sync信号,async_watcher接收信号，从队列取数据,调用data_handle处理数据*/
	ev_timer        heart_watcher;
	time_t          heart_space;					/*心跳间隔，单位s*/
	int             thread_idx;					/*编号*/
	int (*do_heart)(time_t heart_space, int thread_idx);		/*心跳处理函数*/

	int             pfd[2];
	ev_io           rd_watcher;

	/*业务层*/
	char            logfile[512];					/*该线程的日志文件名*/
	int (*init)(void);						/*初始化处理数据时的配置信息*/
	int (*data_handle)(const void *data);				/*对收到的信息进行处理*/
} OTHER_PTHREAD;

void *out_pthread_start(void *sync_wait);

