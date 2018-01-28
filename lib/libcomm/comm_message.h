#pragma once
#include <stdbool.h>

#include "comm_confs.h"
#include "comm_sds.h"

/* 数据包的设置 */
struct comm_package
{
	struct comm_sds raw_data;

	int             packages;				/* 总包数 */
	int             frames_of_package[MAX_COMM_FRAMES];	/* 每个包中帧的多少 */

	int             frames;					/* 总帧数 */
	int             frame_size[MAX_COMM_FRAMES];		/* 每个帧数据大小 */
	int             frame_offset[MAX_COMM_FRAMES];		/* 每个帧的偏移 */
};
/* 发送接收数据的结构体 */
struct comm_message
{
	bool                    canfree;
	int                     fd;		/* 消息对应的描述符 */
	int                     flags;		/* 消息的加密压缩格式 */
	int                     ptype;		/* 消息套接字协议的类型 */
	struct comm_package     package;	/* 消息包分解器 */
};

struct comm_message     *commmsg_make(struct comm_message *message, size_t cap);

/* 拷贝一份comm_message结构体的数据 @dstmsg:拷贝的目的结构体 @srcmsg:拷贝的源结构体 */
void commmsg_copy(struct comm_message *dstmsg, struct comm_message *srcmsg);

/* 清空comm_message内的数据 */
void commmsg_free(struct comm_message *message);

bool commmsg_check(const struct comm_message *message);

void commmsg_sets(struct comm_message *message, int fd, int flags, int ptype);

void commmsg_gets(struct comm_message *message, int *fd, int *flags, int *ptype);

#define commmsg_frame_count(msg)	(msg)->package.frames
#define commmsg_package_count(msg)	(msg)->package.packages

#define commmsg_frame_size(msg, idx)    (msg)->package.frame_size[idx]
#define commmsg_frame_addr(msg, idx)    (&(msg)->package.raw_data.str[(msg)->package.frame_offset[idx]])

char *commmsg_frame_get(struct comm_message *msg, int index, int *size);

int commmsg_frame_set(struct comm_message *msg, int index, int size, char *frame);

int commmsg_frame_del(struct comm_message *msg, int index, int nframe);

void commmsg_print(struct comm_message *message);
