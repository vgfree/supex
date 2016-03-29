#include <signal.h>
#include <time.h>

#include "utils.h"

#include "timport_evcb.h"

#include "timport_task.h"

#define ONE_MINUTE 60.

void timport_signal_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
	x_printf(S, "get a signal : %s", SWITCH_NULL_STR(sys_siglist[w->signum]));

	if ((w->signum == SIGQUIT) || (w->signum == SIGINT)) {
		ev_signal_stop(loop, w);

		ev_break(loop, EVBREAK_ALL);

		x_printf(S, "process has been shuted down by signal [%d].", w->signum);
	}
}

void timport_timer_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	ev_timer_stop(loop, w);

	timport_task_check((void *)loop);

	ev_timer_set(w, ONE_MINUTE, 0.);
	ev_timer_start(loop, w);
}

