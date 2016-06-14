/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_structure.h"

#define set_parameter(type, capacity, cnt,fda )				\
		({							\
		 switch (type) {					\
			case REMAINFD_LISTEN:				\
				cnt = &remainfd->cnt[0];		\
				fda = remainfd->fda[0];			\
				capacity = &remainfd->capacity[0];	\
				break ;					\
			case REMAINFD_READ:				\
				cnt = &remainfd->cnt[1];		\
				fda = remainfd->fda[1];			\
				capacity = &remainfd->capacity[1];	\
				break ;					\
			case REMAINFD_PARSE:				\
				cnt = &remainfd->cnt[2];		\
				fda = remainfd->fda[2];			\
				capacity = &remainfd->capacity[2];	\
				break ;					\
			case REMAINFD_PACKAGE:				\
				cnt = &remainfd->cnt[3];		\
				fda = remainfd->fda[3];			\
				capacity = &remainfd->capacity[3];	\
				break ;					\
			case REMAINFD_WRITE:				\
				cnt = &remainfd->cnt[4];		\
				fda = remainfd->fda[4];			\
				capacity = &remainfd->capacity[4];	\
				break ;					\
			default:					\
				break ;					\
		}							\
		})		

inline bool new_commmsg(struct comm_message** message, int size)
{
	assert(message);

	New(*message);
	if (*message) {
		NewArray((*message)->content, size);
		if ((*message)->content) {
			return true;
		} else {
			Free(*message);
		}
	}
	return false;
}

inline void copy_commmsg(struct comm_message* destmsg, const struct comm_message* srcmsg)
{
	assert(destmsg && srcmsg && destmsg->content && srcmsg->content);
	destmsg->fd = srcmsg->fd;
	destmsg->config = srcmsg->config;
	destmsg->socket_type = srcmsg->socket_type;
	memcpy(&destmsg->package, &srcmsg->package, sizeof(srcmsg->package));
	memcpy(destmsg->content, srcmsg->content, srcmsg->package.dsize);
}

inline void free_commmsg(void* arg)
{
	struct comm_message *message = (struct comm_message*)arg;
	if (message) {
		Free(message->content);
		Free(message);
	}
}


bool init_remainfd(struct remainfd *remainfd)
{
	assert(remainfd);

	int i = 0, j = 0;
	memset(remainfd, 0, sizeof(*remainfd));
	remainfd->capacity[0] = LISTEN_SIZE;	/* REMAINFD_LISTEN类型的fd大小一定，不可以扩容 */

	for(i = 0; i < 5; i++) {
		if (i != 0) {
			remainfd->capacity[i] = EPOLL_SIZE/2;
		}
		remainfd->fda[i] = calloc(remainfd->capacity[i], sizeof(int));
		if (unlikely(!remainfd->fda[i])) {
			for(j = 0; j < i; j++) {
				free(remainfd->fda[j]);
			}
			remainfd->init = false;
		}
	}
	remainfd->init = true;
	return remainfd->init;
}

void restore_remainfd(struct remainfd *remainfd) 
{
	assert(remainfd && remainfd->init);
	int  i = 0;
	int* ptr = NULL;
	//int  capacity = 0;

	/* REMAINFD_LISTEN类型的fd不进行扩容，所以不用恢复 */
	for(i = 1; i < 5; i++) {
		if (remainfd->capacity[i] > EPOLL_SIZE/2 && remainfd->cnt[i] < EPOLL_SIZE/2) {
			ptr = remainfd->fda[i];
			//capacity = remainfd->capacity[i];
			remainfd->fda[i] = realloc(ptr, EPOLL_SIZE/2);
			if (unlikely(!remainfd->fda[i])) {
				remainfd->fda[i] = ptr;
			} else {
				remainfd->capacity[i] = EPOLL_SIZE/2;
			}
		}
	}
	return ;
}

void free_remainfd(struct remainfd *remainfd) 
{
	if (remainfd && remainfd->init) {
		int i = 0;
		for (i = 0; i < 5; i++) {
			free(remainfd->fda[i]);
		}
		remainfd->init = false;
	}
	return ;
}

bool add_remainfd(struct remainfd *remainfd, int fd, int type) 
{
	assert(remainfd && remainfd->init);
	int	i = 0;
	int*	capacity = NULL;
	int*	cnt = NULL;
	int*	fda = NULL;
	bool	flag = true;

	set_parameter(type, capacity, cnt, fda);

	if (unlikely(*capacity < *cnt + 1)) {
		/* 容量不够，则进行扩容 */
		int *ptr = fda;
		fda = realloc(ptr, *capacity + REMAINFD_INCREASE_SIZE);
		if (fda) {
			*capacity += REMAINFD_INCREASE_SIZE;
		} else {
			/* 扩容失败,直接返回false */
			fda = ptr;
			return false;
		}

	}
	for (i = 0; i < *cnt; i++) {
		if (fda[i] == fd) {
			flag = false;
			break ;
		}
	}
	if (flag) {
		fda[*cnt] = fd;
		*cnt += 1;
	}
	return flag;
}

void del_remainfd(struct remainfd *remainfd, int fd, int type)
{
	assert(remainfd && remainfd->init);
	int	i = 0;
	int*	capacity = NULL;
	int*	cnt = NULL;
	int*	fda = NULL;

	set_parameter(type, capacity, cnt, fda);
	for (i = *cnt-1; i > -1; i--) {
		if (fda[i] == fd) {
			if (i != *cnt-1) {
				memmove(&(fda[i]), &(fda[i+1]), (*cnt-(i+1))*sizeof(int));
			}
			*cnt -= 1;
		}
	}
	return ;
}
