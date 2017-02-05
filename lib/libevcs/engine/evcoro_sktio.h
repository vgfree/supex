//
//  evcoro_io.h
//  libevcoro
//
//  Created by 钱慧奇 on 16/12/7.
//  Copyright © 2016年 baoxue. All rights reserved.
//

#ifndef ev_coro_io_h
#define ev_coro_io_h

#include "evcoro_scheduler.h"

#if __cplusplus
extern "C" {
#endif

/*
 * return:
 * 0 is ok read len,
 * -1 is err happend.
 */
int evcoro_sktio_read(struct evcoro_scheduler *scheduler, int sockfd, void *buf, unsigned int len, uint32_t timeout_repeat);

/*
 * return:
 * 0 is ok write len,
 * -1 is err happend.
 */
int evcoro_sktio_write(struct evcoro_scheduler *scheduler, int sockfd, void *buf, unsigned int len, uint32_t timeout_repeat);

#if __cplusplus
}
#endif
#endif	/* ev_coro_io_h */

