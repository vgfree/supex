//
//  data_model.c
//  supex
//
//  Created by 周凯 on 15/12/5.
//  Copyright © 2015年 zk. All rights reserved.
//

#include "zmq.h"
#include "data_model.h"
#include "zmq_wrap.h"

void stop_proc(struct proc *proc)
{
	return_if_fail(ISOBJ(proc));

	ATOMIC_SET(&proc->stat, PROC_STAT_STOP);

	if (proc->coroloop) {
		evcoro_stop(proc->coroloop);
	}

	return_if_fail(ISOBJ(proc->frame));

	ATOMIC_SET(&proc->frame->stat, FRAME_STAT_STOP);
}

void kill_proc(struct proc *proc)
{
	return_if_fail(ISOBJ(proc));

	ATOMIC_SET(&proc->stat, PROC_STAT_STOP);

	if (proc->coroloop) {
		evcoro_stop(proc->coroloop);
	}

	return_if_fail(ISOBJ(proc->frame));

	ATOMIC_SET(&proc->frame->stat, FRAME_STAT_STOP);

	if ((proc->tid > 0) && (proc->stat != PROC_STAT_STOP)) {
		pthread_cancel(proc->ptid);
	}
}

void iohandle_destroy(struct iohandle *io)
{
	return_if_fail(io);

	if (((io->type == SOCK_ZMQ_BIND) ||
		(io->type == SOCK_ZMQ_CONN))) {
		if (io->zmq.skt) {
			if (io->name[0]) {
				if (io->type == SOCK_ZMQ_BIND) {
					zmq_unbind(io->zmq.skt, io->name);
				} else {
					zmq_disconnect(io->zmq.skt, io->name);
				}
			}

			/*延长两秒关闭*/
			if (io->linger > -1) {
				set_zmqopt(io->zmq.skt, ZMQ_LINGER, io->linger);
			}

			zmq_close(io->zmq.skt);
		}

		if (io->type == SOCK_ZMQ_BIND) {
			zmq_term(io->zmq.ctx);
		}
	} else {
		close(io->http);
	}

	free(io);
}

