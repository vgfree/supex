/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_PIPE_H__
#define __COMM_PIPE_H__

#include "comm_utils.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

struct comm_pipe
{
	bool    init;			/* 用于判断此结构体是否被初始化 */
	int     wfd;			/* PIPE打开用于读的描述符 */
	int     rfd;			/* PIPE打开用于写的描述符 */
};

/* 创建一个管道 */
bool commpipe_create(struct comm_pipe *commpipe);

/* 关闭一个管道 */
void commpipe_destroy(struct comm_pipe *commpipe);

#ifdef __cplusplus
}
#endif
#endif	/*#ifndef __COMM_PIPE_H__*/

