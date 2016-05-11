/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_PARSE_H__
#define __COMM_PARSE_H__

#include "comm_structure.h"
#include "./mfptp_protocol/mfptp_parse.h"

#ifdef __cplusplus
extern "C" {
#endif


     
bool parse_data(struct comm_data *commdata);


     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_PARSE_H__ */
