/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "communication.h"

#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <dirent.h>


static void * _start_new_pthread(void* usr);

//往epfd里面添加一个需要监控的fd
static inline bool _add_epoll(struct comm *commctx, int fd, unsigned int flag )
{
	assert(commctx);

	int			retval = 0;
	struct epoll_event	event = {};

	if( likely(commctx->watchcnt < EPOLL_SIZE) ){
		event.data.fd = fd;
		event.events = flag;
		retval = epoll_ctl(commctx->epfd, EPOLL_CTL_ADD, fd, &event);
		if( likely(retval == 0)){
			commctx->watchcnt ++;
			return true;
		}
	}

	return false;
}

//从epfd里面删除一个已经监控的fd
static inline bool  _del_epoll(struct comm *commctx, int fd, unsigned int flag )
{
	assert(commctx);
	int			retval = 0;
	struct epoll_event	event = {};
	event.data.fd = fd;
	event.events = EPOLLIN;
	retval = epoll_ctl(commctx->epfd, EPOLL_CTL_DEL, fd, &event);
	if( likely(retval == 0)){
		return true;
	}
	return  false;
}

static inline bool _get_portinfo(int fd, int type, struct portinfo *portinfo)
{
	int	retval = -1;

	retval = get_address(fd,  portinfo->addr, (size_t)sizeof(portinfo->addr));
	if( unlikely(retval == -1) ){
		return false;
	}
	portinfo->port = get_port(fd);
	if( unlikely(portinfo->port == -1) ){
		return false;
	}
	portinfo->fd = fd;
	portinfo->type = type;

	return true;
}

//epoll_wait监听事件发生时的回调函数:真正的发送或者读取数据recv read
static void _finished_event(struct comm* commctx, int fd, int flag)
{
	ssize_t bytes = 0;
	int miou = 1024;
	char buff[miou];
	memset(buff, 0 , miou);
	if( commctx->data[fd ]){
		if( flag == EPOLLIN ){				//可读
			bytes = read(fd, buff, miou);
			if(bytes >  0){ 
				commcache_append(&commctx->data[fd]->recv_buff, buff, bytes);
			}else if( bytes == 0){			//对端已关闭
				comm_close(commctx, fd);
			}else{
				printf("read from %d fd failed\n", fd);
			}
		}else if( flag == EPOLLOUT ){						//可写
//			commqueue_pull();
			bytes = write(fd, buff, strlen(buff));
			if( unlikely(bytes < 0) ){
				printf("write from %d fd failed\n", fd);
			}
		}else{ 
			//accpet的类型
		}
		if (commctx->data[fd ]->finishedcb.callback) {
		  commctx->data[fd ]->finishedcb.callback(commctx, fd, commctx->data[fd ]->finishedcb.usr);
		}
	}
}

/*关闭父进程所有打开的文件描述符， fd:此描述符不关闭*/
static bool  _close_all_fd(int pfd)
{
	int		retval = 0;
	int		rewind = 0;
	int		fd = 0;
	DIR*		dir = NULL;
	struct dirent	*entry, _entry;

	if( unlikely(!(dir = opendir("/dev/fd"))) ){
		return false;
	}

	while (1) {

		retval = readdir_r(dir, &_entry, &entry);
		if( unlikely(retval != 0) ){
			closedir(dir);
			return false;
		}
		if (entry == NULL) {
			if (!rewind){
				break;
			}
			rewinddir(dir);
			rewind = 0;
			continue;
		}
		if (entry->d_name[0] == '.'){
			continue;
		}
		fd = atoi(entry->d_name);
		if ( unlikely(dirfd(dir) == fd || fd == 1 || fd == 0 ||fd == 2 || fd == pfd) ){
			continue;
		}
		close(fd);
		rewind = 1;
	}
	closedir(dir);
	return true;
}
//初始化一个fd的数据结构体，并将此fd添加到epoll的监控中
static  bool  _commdata_init(struct comm* commctx,
							 int fd,
							 struct portinfo portinfo,
							 struct cbinfo  *finishedcb, int flag)
{
	assert(commctx);
	int nodesize = 10; //待定
	bool retval = false;

	struct comm_data *commdata = malloc(sizeof(struct comm_data));
	commctx->data[fd] = commdata;
	if( unlikely(!commctx) ){
		goto error;
	}
	retval = commqueue_init(&commdata->recv_queue, QUEUE_CAPACITY, nodesize);
	if( unlikely(!retval) ){
		goto error;
	}
	retval = commqueue_init(&commdata->send_queue, QUEUE_CAPACITY, nodesize);
	if( unlikely(!retval) ){
		goto error;
	}
	retval = commcache_init(&commdata->recv_buff, CACHE_SIZE);
	if( unlikely(!retval) ){
		goto error;
	}

	retval = commcache_init(&commdata->send_buff, CACHE_SIZE);
	if( unlikely(!retval) ){
		goto error;
	}

	retval = _add_epoll(commctx, fd, flag);
	if( unlikely(!retval) ){
		goto error;
	}

	commdata->finishedcb = *finishedcb;
	commdata->commctx = commctx;
	commdata->portinfo = portinfo;
	return true;

error:
	commqueue_destroy(&commdata->recv_queue);
	commqueue_destroy(&commdata->send_queue);
	
	commcache_free(&commdata->recv_buff);
	commcache_free(&commdata->send_buff);
	Free(commdata);
	return false;
}

static  void _commdata_destroy(struct comm_data *commdata)
{
	if( likely(commdata) ){
		commqueue_destroy(&commdata->recv_queue);
		commqueue_destroy(&commdata->send_queue);
		
		commcache_free(&commdata->recv_buff);
		commcache_free(&commdata->send_buff);

		_del_epoll(commdata->commctx, commdata->portinfo.fd, 0);
		Free(commdata);
	}

	return ;
}


int comm_socket(struct comm *commctx, char *host, char *server, struct cbinfo finishedcb, int type)
{
	assert(commctx && host && server);

	int			fd = -1;
	int			flag = 0;
	bool			retval = false;

	if (type == COMM_BIND) {
		flag = EPOLLIN; //监听读事件
		fd = socket_queuen(host, server);
		commctx->listenfd = fd;
	} else {
		flag =  EPOLLOUT; //监听写事件
		fd = socket_connect(host, server);
	}

	if( unlikely(fd < 0) ){
		return fd;
	}

	struct portinfo portinfo = {};

	if (unlikely(!_get_portinfo(fd, type, &portinfo)) ){
		close(fd);
		return -1;
	}
	_commdata_init(commctx, fd, portinfo, &finishedcb, flag);
	if (unlikely(!commctx)) {
		close(fd);
		return -1;
	}
	ATOMIC_SET(&commctx->stat, COMM_STAT_RUN);

	return fd;
}

//将用户传进来的数据放入到发送缓冲区中保存起来:返回-1失败
int comm_send(struct comm *commctx, int fd, const char *buff, int size)
{
	assert(commctx && buff);

	int retval = -1;
	if( likely(commctx->data[fd ]) ){
		retval = commcache_append(&commctx->data[fd ]->send_buff, buff, size);
		if( unlikely(retval < 0) ){
			printf("commcache_append failed in comm_send\n");
		}
		return 0;
	}
	return retval;
}

//将已经解析好的数据返回给用户:返回-1失败
int comm_recv(struct comm *commctx, int fd, char *buff, int size)
{
	assert(commctx && buff);

	int retval = -1;
	if (likely(commctx->data[fd ])) {
		retval = commqueue_pull(&commctx->data[fd ]->recv_queue, buff);
		if (unlikely(!retval)) {
			printf("commqueue_pull failed in comm_recv\n");
		}
		return 0;
	}
	return retval;
}

//设置epoll_wait的超时时间以及超时时的回调函数
void comm_settimeout(struct comm *commctx, int timeout, CommCB callback, void *usr)
{
	commctx->timeoutcb.timeout = timeout;
	commctx->timeoutcb.callback  = callback;
	commctx->timeoutcb.usr = usr;
}

//关闭指定的描述符
void comm_close(struct comm *commctx, int fd)
{
	if( likely(commctx) ){
		if( likely(commctx->data[fd ])){
			_commdata_destroy(commctx->data[fd ]);
			commctx->data[fd ] = NULL;
		}
		close(fd);
	}
	return ;
}





//修改一个epfd里面的已经监控的fd事件
static inline bool  _mod_epoll(struct comm *commctx, int fd, unsigned int flag )
{
	assert(commctx);

	int			retval = 0;
	struct epoll_event	event = {};
	event.data.fd = fd;
	event.events = flag;
	retval = epoll_ctl(commctx->epfd, EPOLL_CTL_MOD, fd, &event);
	if( likely(retval == 0)){
		return true;
	}
	return  false;
}

//epoll等待事件的发生
static int  _epoll_wait(struct comm *commctx)
{
	assert(commctx);

	int			nfds = 0;
	int			n = 0;
	int			fd = -1;
	bool			retval = false;
	struct epoll_event	events[EPOLL_SIZE] = {};

	nfds = epoll_wait(commctx->epfd, events, commctx->watchcnt, commctx->timeoutcb.timeout);
	if( likely(nfds > 0)){
		for (n = 0; n < nfds; ++n){
			if( unlikely(events[n].data.fd < -1)){
				continue ;
			}
			if ( commctx->listenfd != -1 && events[n].data.fd == commctx->listenfd){
				fd = accept(commctx->listenfd, NULL, NULL);
				if( unlikely(fd < 0) ){
					if( likely(errno == EAGAIN || errno == EWOULDBLOCK) ){
						continue ;
					}else{
						return -1;
					}
				}
				struct portinfo portinfo = {};
				if( unlikely(!_get_portinfo(fd, COMM_ACCEPT, &portinfo)) ){
					close(fd);
					return -1;
				}
				retval = _commdata_init(commctx, fd, portinfo, NULL, EPOLLIN);
				if( unlikely(!retval) ){
					close(fd); //无法对此描述符进行监控，则关闭此描述符
					continue;
				}else{
					_finished_event(commctx, fd, EPOLLIN);
				}
			}else if( events[n].events & EPOLLIN ){	//接收到数据，读socket
				fd = events[n].data.fd;
				_finished_event(commctx, fd, EPOLLIN);
				_mod_epoll(commctx, events[n].data.fd, EPOLLOUT);
			}
			else if( events[n].events & EPOLLOUT ){ /*对应的描述符可写，即套接口缓冲区有缓冲区可写*/
				fd = events[n].data.fd;
				_finished_event(commctx, fd, EPOLLOUT);
				_mod_epoll(commctx, events[n].data.fd, EPOLLIN);
			}
		}  
	}else{
		//epoll_wait 超时
		if( likely(errno == EINTR)){
			/*if( commctx->cbinfo.eventcb){
				_timeout_event(commctx);
				fd = -1;
			}*/
		}
	}
	return fd;
}

//epoll_wait超时时的回调函数:解析和打包数据
static void _timeout_event(struct comm* commctx)
{
	int fd = 0;
	int counter = commctx->watchcnt;
	while(counter){
		if( likely(commctx->data[fd ]) ){
			//接收缓冲区里面存在数据需要解析
			if( likely(commctx->data[fd ]->recv_buff.end - commctx->data[fd]->recv_buff.start > 0) ){
				int size = 0; //解析数据的字节数
				size = parse(commctx->data[fd ]->recv_buff); //待定函数
				//将解析完的数据push到接收的队列里面
		//		commqueue_push();
				commcache_deccnt(&commctx->data[fd ]->recv_buff, size);
			}
			//发送缓冲区里面存在数据需要打包
			if( likely(commctx->data[fd]->send_buff.end - commctx->data[fd]->recv_buff.start > 0) ){
				package(commctx->data[fd]->send_buff);//待定函数
				//将打包好的数据push到发送的队列里面
		//		commqueue_push();
			}
			counter--;
		}
		fd++;
	}
	if( likely(commctx->timeoutcb.callback)){ //调用用户层的回调函数
		commctx->timeoutcb.callback(commctx, -1, commctx->timeoutcb.usr);
	}
}

//一个新的线程开始运行
static void * _start_new_pthread(void* usr)
{
	assert(usr);
	int		fd = -1;
	bool		retval = -1;
	struct comm*	commctx = (struct comm*)usr;

	if( unlikely(!(retval = _close_all_fd(commctx->epfd))) ){
		return NULL;
	}

	futex_cond_wait(&commctx->stat, COMM_STAT_RUN, -1);
	while(1){
		//线程状态为STOP的时候则，将状态设置为NONE，返回真，则代表设置成功，退出循环
		if( unlikely(ATOMIC_CASB(&commctx->stat, COMM_STAT_STOP, COMM_STAT_NONE)) ){
			break;
		}
		if(commctx->timeoutcb.timeout <= 0){ //用户没有设置epoll_wait超时，则使用默认超时时间
			commctx->timeoutcb.timeout = TIMEOUTED;
		}
		retval = _epoll_wait(commctx);
	}
}

struct comm* comm_ctx_create(int epollsize)
{
	int		retval = -1;
	struct comm*	commctx = malloc(sizeof(struct comm));

	if( unlikely(!commctx) ){
		printf("no more memory, calloc commctx failed\n");
		return commctx;
	}

	commctx->epfd = epoll_create(epollsize > 0 ? epollsize :EPOLL_SIZE);
	if( unlikely(commctx->epfd < 0) ){
		Free(commctx);
		return commctx;
	}
	commctx->listenfd = -1;
	retval = pthread_create(&commctx->ptid, NULL,  _start_new_pthread, (void *)commctx);
	if( unlikely(retval != 0) ){
		close(commctx->epfd);
		Free(commctx);
		return commctx;
	}
	commctx->stat = COMM_STAT_INIT; 

	return commctx;
}

void comm_ctx_destroy(struct comm* commctx)
{
	int fd = 0;
	if( likely(commctx) ){
		//原子设置线程的状态
		ATOMIC_SET(&commctx->stat, COMM_STAT_STOP);
		futex_cond_wait(&commctx->stat, COMM_STAT_NONE, -1);
		while(commctx->watchcnt){
			if( likely(commctx->data[fd ]) ){
				comm_close(commctx, fd);
			}
			fd ++;
		}
		close(commctx->epfd);
		pthread_join(commctx->ptid, NULL);
		Free(commctx);
	}
}


