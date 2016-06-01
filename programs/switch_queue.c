#include "switch_queue.h"

bool switch_queue_init(struct switch_queue_info *p_stat,
	unsigned int                            task_size,
	volatile long                           *major_have,
	QUEUE_ENTITY_CALL                       major_push,
	QUEUE_ENTITY_CALL                       major_pull,
	volatile long                           *minor_have,
	QUEUE_ENTITY_CALL                       minor_push,
	QUEUE_ENTITY_CALL                       minor_pull)
{
	memset(p_stat, 0, sizeof(struct switch_queue_info));
	supex_node_init(&p_stat->temp, NULL, task_size);
	supex_task_init(&p_stat->swap, task_size, MAX_SWITCH_QUEUE_SWAP_SIZE);
	p_stat->major_have = major_have;
	p_stat->major_push = major_push;
	p_stat->major_pull = major_pull;

	p_stat->minor_have = minor_have;
	p_stat->minor_push = minor_push;
	p_stat->minor_pull = minor_pull;
	return true;
}

bool switch_queue_push(struct switch_queue_info *p_stat, struct supex_task_node *p_node, ...)
{
	bool            ok = false;
	va_list         ap;
	int             new_mark = 0;
	unsigned int    new_shift = 0;

	do {
		// ---->use queue
		new_mark = MARK_USE_MAJOR;

		if (p_stat->mark_report == MARK_USE_MAJOR) {
			p_node->base->shift = p_stat->shift_report;
		} else {
			p_node->base->shift = p_stat->shift_report + 1;
		}

		new_shift = p_node->base->shift;

		va_start(ap, p_node);
		ok = p_stat->major_push(p_stat, p_node, &ap);
		va_end(ap);

		if (ok) {
			x_printf(D, "push queue ok!");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;

			if (p_stat->major_have) {
				AO_INC(p_stat->major_have);
			}

			break;
		} else {
			x_printf(W, "push queue failed!");
		}

		// ---->use ucmq
		new_mark = MARK_USE_MINOR;

		if (p_stat->mark_report == MARK_USE_MINOR) {
			p_node->base->shift = p_stat->shift_report;
		} else {
			p_node->base->shift = p_stat->shift_report + 1;
		}

		new_shift = p_node->base->shift;
		va_start(ap, p_node);
		ok = p_stat->minor_push(p_stat, p_node, &ap);
		va_end(ap);

		if (ok) {
			x_printf(D, "push ucmq ok!");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;

			if (p_stat->major_have) {
				AO_INC(p_stat->major_have);
			}

			break;
		} else {
			x_printf(E, "push ucmq failed!");
		}
	} while (0);
	return ok;
}

bool switch_queue_pull(struct switch_queue_info *p_stat, struct supex_task_node *p_node, ...)
{
	bool    ok = false;
	long    have = 0;
	va_list ap;

	switch (p_stat->step_lookup)
	{
		case 0:
			/*init vms step*/
			va_start(ap, p_node);
			ok = p_stat->major_pull(p_stat, p_node, &ap);
			va_end(ap);

			if (ok) {
				x_printf(D, "pull queue ok!");

				if (p_stat->major_have) {
					AO_DEC(p_stat->major_have);
				}

				p_stat->step_lookup++;
			}

			break;

		case 1:
			/*done old step*/
			va_start(ap, p_node);
			ok = p_stat->minor_pull(p_stat, p_node, &ap);
			va_end(ap);

			if (ok) {
				if (p_node->base->shift == 1) {
					assert(supex_task_push(&p_stat->swap, p_node->data));	// push l
					p_stat->step_lookup++;
					ok = false;
				} else {
					// do nothing, is old task
				}
			} else {
				p_stat->step_lookup++;
			}

			break;

		case 2:
SCHEDULE_LOOKUP_START:

			/*done new step*/
			if (p_stat->mark_lookup == MARK_USE_MAJOR) {
				va_start(ap, p_node);
				ok = p_stat->major_pull(p_stat, p_node, &ap);
				va_end(ap);

				if (ok) {
					if (p_node->base->shift == p_stat->shift_lookup) {
						// do nothing, is next task
						if (p_stat->major_have) {
							AO_DEC(p_stat->major_have);
						}
					} else {
						/*not first push then pop,it will get the push task in one loop.*/
						supex_node_copy(&p_stat->temp, p_node);
						ok = supex_task_pull(&p_stat->swap, p_node->data);	// pull l

						if (ok) {
							if (p_stat->minor_have) {
								AO_DEC(p_stat->minor_have);
							}
						}

						assert(supex_task_push(&p_stat->swap, p_stat->temp.data));	// push l
						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_MINOR;

						if (!ok) {
							goto SCHEDULE_LOOKUP_START;
						}
					}
				} else {
					if (p_stat->minor_have) {
						have = AO_GET(p_stat->minor_have);
					}

					if (!p_stat->minor_have || (have > 0)) {
						ok = supex_task_pull(&p_stat->swap, p_node->data);	// pull l

						if (ok) {
							if (p_stat->minor_have) {
								AO_DEC(p_stat->minor_have);
							}
						}

						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_MINOR;

						if (!ok) {
							goto SCHEDULE_LOOKUP_START;
						}
					} else {
						// do nothing, no task
					}
				}
			} else {
				va_start(ap, p_node);
				ok = p_stat->minor_pull(p_stat, p_node, &ap);
				va_end(ap);

				if (ok) {
					if (p_node->base->shift == p_stat->shift_lookup) {
						// do nothing, is next task
						if (p_stat->minor_have) {
							AO_DEC(p_stat->minor_have);
						}
					} else {
						supex_node_copy(&p_stat->temp, p_node);
						ok = supex_task_pull(&p_stat->swap, p_node->data);	// pull l

						if (ok) {
							if (p_stat->major_have) {
								AO_DEC(p_stat->major_have);
							}
						}

						assert(supex_task_push(&p_stat->swap, p_stat->temp.data));	// push l
						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_MAJOR;

						if (!ok) {
							goto SCHEDULE_LOOKUP_START;
						}
					}
				} else {
					if (p_stat->major_have) {
						have = AO_GET(p_stat->major_have);
					}

					if (!p_stat->major_have || (have > 0)) {
						ok = supex_task_pull(&p_stat->swap, p_node->data);	// pull l

						if (ok) {
							if (p_stat->major_have) {
								AO_DEC(p_stat->major_have);
							}
						}

						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_MAJOR;

						if (!ok) {
							goto SCHEDULE_LOOKUP_START;
						}
					} else {
						// do nothing, no task
					}
				}
			}

			break;

		default:
			break;
	}
	return ok;
}

