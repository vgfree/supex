/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/23.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_DATA_H__
#define __COMM_DATA_H__

#include "comm_structure.h"

#ifdef __cplusplus
extern "C" {
#endif


/* 初始化一个fd的数据结构体struct comm_data */
bool commdata_init(struct comm_data **commdata, struct comm_tcp* commtcp,  struct cbinfo*  finishedcb);

/* 销毁一个fd的数据结构体struct comm_data*/
void commdata_destroy(struct comm_data *commdata);

/* 开始打包一个fd的数据 */
bool  commdata_package(struct comm_data *commdata, struct comm_event *commevent);

/* 开始解析一个fd的数据 */
bool commdata_parse(struct comm_data *commdata, struct comm_event *commevent, bool remainfd);





#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_DATA_H__ */
