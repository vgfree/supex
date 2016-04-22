/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef	__COMM_PIPE_H__
#define __COMM_PIPE_H__

#include "comm_utils.h"
#include <sys/types.h>
#include <sys/stat.h>


#ifdef __cplusplus
extern "C" {
#endif

#define	MAXPATHLEN	128

struct comm_pipe {
	bool init;			/* 用于判断此结构体是否被初始化 */
	int  wfd;			/* PIPE打开用于读的描述符 */
	int  rfd;			/* PIPE打开用于写的描述符 */
};

bool commpipe_init(struct comm_pipe *commpipe);

void commpipe_destroy(struct comm_pipe *commpipe);

int commpipe_read(struct comm_pipe *commpipe, void *buff, int size);

int commpipe_write(struct comm_pipe *commpipe, void *buff, int size);


#ifdef __cplusplus
	}
#endif 

#endif /*#ifndef __COMM_PIPE_H__*/
