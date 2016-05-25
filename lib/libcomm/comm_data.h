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


bool commdata_init(struct comm_data **commdata, struct comm_tcp* commtcp,  struct cbinfo*  finishedcb);

void commdata_destroy(struct comm_data *commdata);

bool commdata_package(struct comm_data *commdata, struct comm_event *commevent);

bool commdata_parse(struct comm_data *commdata, struct comm_event *commevent);





#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_DATA_H__ */
