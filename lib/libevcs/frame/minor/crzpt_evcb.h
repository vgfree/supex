#pragma once

#include <ev.h>

#include "engine/adopt_tasks/adopt_task.h"

void crzpt_all_task_hit(struct adopt_task_node *task, bool synch);

void crzpt_one_task_hit(struct adopt_task_node *task, bool synch);

void crzpt_idle_cb(struct ev_loop *loop, ev_idle *w, int revents);

