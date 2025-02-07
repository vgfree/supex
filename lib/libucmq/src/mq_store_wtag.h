/*
 *  Copyright (c) 2013 UCWeb Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * You may obtain a copy of the License at
 *
 *       http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Email: osucmq@ucweb.com
 * Author: ShaneYuan
 */

#ifndef  __MQ_STORE_wtag_H__
#define  __MQ_STORE_wtag_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include "mq_util.h"

//////////////////////////////////////////////////////////////////////////

#define READ_OPT_TIMEOUT        1	/* read file operation time out */
#define WRITE_OPT_TIMEOUT       1	/* write file operation time out */

/* Length of the wtag item */
#define WTAG_ITEM_LEN           40	/* wtag item len */
#define WCOUNT_LEN              20	/* readed item number */
#define WPOS_LEN                10	/* write file pos */
#define QUEUE_MAX_SIZE_LEN      10	/* queue max size */
#define WTAG_FILE_HEAD_LEN      0	/* wtag file head */

//////////////////////////////////////////////////////////////////////////

int mq_sm_wtag_open_next_file(const char *file_name);

bool mq_sm_wtag_open_file(mq_queue_t *mq_queue, queue_file_t *queue_file);

bool mq_sm_wtag_close_file(mq_queue_t *mq_queue);

bool mq_sm_wtag_read_item(mq_queue_t *mq_queue);

bool mq_sm_wtag_write_item(mq_queue_t *mq_queue);

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif	/* __MQ_STORE_WTAG_H__ */

