//
//  spx_common.h
//  supex
//
//  Created by 周凯 on 15/7/16.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#ifndef __supex_common_h__
#define __supex_common_h__

/*==============================================================================================*
*		常用最值宏									*
*==============================================================================================*/
#define MAX_FILE_NAME_SIZE      32
#define MAX_FILE_PATH_SIZE      512
#define MAX_API_COUNTS          255

#ifdef _mttptest
  #define MAX_MCB_COUNTS        255
#endif

#define MAX_CMD_COUNTS          255
#define MAX_API_NAME_LEN        63
#define MAX_CONNECT             20000
#define MAX_LIMIT_FD            60000	// MUST > MAX_CONNECT
#define MAX_DEF_LEN             (1024 * 1024 * 10)
#define DEFAULT_REQ_SIZE        32768
#define MAX_TEXT_SIZE           (4096)

/*==============================================================================================*
*		time function									*
*==============================================================================================*/
#define ONE_DAY_TIMESTAMP       (24 * 60 * 60)
#endif	/* ifndef __supex_common_h__ */

