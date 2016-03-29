//
//  data_queue.h
//  supex
//
//  Created by 周凯 on 15/9/16.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef _supex_data_queue_h
#define _supex_data_queue_h

#include "data_model.h"
__BEGIN_DECLS
/* --------             */
void queue_init(struct queue *queue, struct cfg *cfg);

void queue_finally(struct queue *queue);

/* --------             */

/*
 * 检查队列是否满或空，如果为成立，则阻塞一会
 */
void queue_check_isfull(struct queue *queue);

void queue_check_isempty(struct queue *queue);

__END_DECLS
/* --------             */
#endif	/* _supex_data_queue_h */

