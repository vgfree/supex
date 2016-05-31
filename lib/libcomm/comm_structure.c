/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_structure.h"

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

	memset(remainfd, 0, sizeof(*remainfd));
	remainfd->capacity = EPOLL_SIZE/6;

	remainfd->wfda = calloc(remainfd->capacity, sizeof(int));
	if (unlikely(!remainfd->wfda)) {
		return false;
	}
	remainfd->rfda = calloc(remainfd->capacity, sizeof(int));
	if (unlikely(!remainfd->rfda)) {
		free(remainfd->wfda);
		return false;
	}
	remainfd->packfda = calloc(remainfd->capacity, sizeof(int));
	if (unlikely(!remainfd->packfda)) {
		free(remainfd->wfda);
		free(remainfd->rfda);
		return false;
	}
	remainfd->parsfda = calloc(remainfd->capacity, sizeof(int));
	if (unlikely(!remainfd->parsfda)) {
		free(remainfd->wfda);
		free(remainfd->rfda);
		free(remainfd->packfda);
		return false;
	}
	remainfd->init = true;
	return true;
}

bool restore_remainfd(struct remainfd *remainfd) 
{
	assert(remainfd && remainfd->init);
	int* ptr = NULL;
	int capacity = remainfd->capacity;
	if (remainfd->capacity > EPOLL_SIZE/6) {
		remainfd->capacity = EPOLL_SIZE/6;
		ptr = remainfd->wfda;
		remainfd->wfda = realloc(ptr, remainfd->capacity);
		if (unlikely(!remainfd->wfda)) {
			remainfd->wfda = ptr;
			return false;
		}
		ptr = remainfd->rfda;
		remainfd->rfda = realloc(ptr, remainfd->capacity);
		if (unlikely(!remainfd->rfda)) {
			remainfd->rfda = ptr;
			return false;
		}
		ptr = remainfd->packfda;
		remainfd->packfda = realloc(ptr, remainfd->capacity);
		if (unlikely(!remainfd->packfda)) {
			remainfd->packfda = ptr;
			return false;
		}
		ptr = remainfd->parsfda;
		remainfd->parsfda = realloc(ptr, remainfd->capacity);
		if (unlikely(!remainfd->parsfda)) {
			remainfd->parsfda = ptr;
			return false;
		}
	}
	return true;
}

void free_remainfd(struct remainfd *remainfd) 
{
	if (remainfd && remainfd->init) {
		free(remainfd->wfda);
		free(remainfd->rfda);
		free(remainfd->packfda);
		free(remainfd->parsfda);
		remainfd->init = false;
	}
	return ;
}

bool add_remainfd(struct remainfd *remainfd, int fd, int type) 
{
	assert(remainfd && remainfd->init && fd > 0);
	int i = 0;
	int* cnt = 0;
	int **fda = NULL;
	if (type == REMAINFD_READ) {
		cnt = &remainfd->rcnt;
		fda = &remainfd->rfda;
	} else if (type == REMAINFD_WRITE) {
		cnt = &remainfd->wcnt;
		fda = &remainfd->wfda;
	} else if (type == REMAINFD_PARSE) {
		cnt = &remainfd->parscnt;
		fda = &remainfd->parsfda;
	} else {
		cnt = &remainfd->packcnt;
		fda = &remainfd->packfda;
	}

	if (*cnt == 0) {
		(*fda)[*cnt] = fd;
		*cnt += 1;
		return true;
	} else if (*cnt == 1) {
		if ((*fda)[0] != fd) {
			(*fda)[1] = fd;
			*cnt += 1;
		}
		return true;
	}

	if (unlikely(remainfd->capacity < *cnt + 1)) {
		/* 扩容的时候会同时全部扩容 */
		int *ptr = remainfd->wfda;
		remainfd->wfda = realloc(ptr, remainfd->capacity + REMAINFD_INCREASE_SIZE);
		if (unlikely(!remainfd->wfda)) {
			remainfd->wfda = ptr;
			return false;
		}
		ptr = remainfd->rfda;
		remainfd->rfda = realloc(ptr, remainfd->capacity + REMAINFD_INCREASE_SIZE);
		if (unlikely(!remainfd->rfda)) {
			remainfd->rfda = ptr;
			return false;
		}
		ptr = remainfd->packfda;
		remainfd->packfda = realloc(ptr, remainfd->capacity + REMAINFD_INCREASE_SIZE);
		if (unlikely(!remainfd->packfda)) {
			remainfd->packfda = ptr;
			return false;
		}
		ptr = remainfd->parsfda;
		remainfd->parsfda = realloc(ptr, remainfd->capacity + REMAINFD_INCREASE_SIZE);
		if (unlikely(!remainfd->parsfda)) {
			remainfd->parsfda = ptr;
			return false;
		}
	}
	for (i = 0; i < *cnt; i++) {
		if ((*fda)[i] < fd && (*fda)[i+1] > fd ) {
			if (i+1 != *cnt) {
				memmove(&((*fda)[i+2]), &((*fda)[i+1]), (*cnt-(i+1))*sizeof(int));
			}
			(*fda)[i+1] = fd;
			*cnt += 1;
		}
	}
	return true;
}

void del_remainfd(struct remainfd *remainfd, int fd, int type)
{
	assert(remainfd && fd > 0);
	int   i = 0;
	int*  cnt = 0;
	int** fda = NULL;
	if (type == REMAINFD_READ) {
		cnt = &remainfd->rcnt;
		fda = &remainfd->rfda;
	} else if (type == REMAINFD_WRITE) {
		cnt = &remainfd->wcnt;
		fda = &remainfd->wfda;
	} else if (type == REMAINFD_PARSE) {
		cnt = &remainfd->parscnt;
		fda = &remainfd->parsfda;
	} else {
		cnt = &remainfd->packcnt;
		fda = &remainfd->packfda;
	}

	if (*cnt > 0) {
		for (i = *cnt - 1; i > -1; i--) {
			if ( (*fda)[i] == fd ) {
				if (i != *cnt -1) {
					memmove(&((*fda)[i]), &((*fda)[i+1]), (*cnt-(i+1))*sizeof(int));
				}
				*cnt -= 1;
			}
		}
	}
}

