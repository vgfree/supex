#ifndef _FREE_QUEUE_LIST_H_
#define _FREE_QUEUE_LIST_H_

#include "libmini.h"

#include <stdint.h>

/*==============================================================================================*
 * *       free queue                                  *
 * *==============================================================================================*/

struct free_queue_list
{
	unsigned int    max;
	unsigned int    all;
	unsigned int    dsz;
	unsigned int    isz;
	unsigned int    osz;
	AO_T            tasks;	/*tasks > 0 时代表有新增任务*/
	void            *slots;
	unsigned int    head;
	unsigned int    tail;
	AO_SpinLockT    w_lock;
	AO_SpinLockT    r_lock;
};

void free_queue_init(struct free_queue_list *list, unsigned int dsz, unsigned int max);

bool free_queue_push(struct free_queue_list *list, void *data);

bool free_queue_pull(struct free_queue_list *list, void *data);
#endif /* ifndef _FREE_QUEUE_LIST_H_ */

