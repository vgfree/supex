#pragma once

#include <ev.h>

#include "mfptp_task.h"

void mfptp_all_task_hit(struct mfptp_task_node *task, bool synch, int mark);

void mfptp_one_task_hit(struct mfptp_task_node *task, bool synch, int mark);

void mfptp_accept_cb(struct ev_loop *loop, ev_io *w, int revents);

void mfptp_prepare_cb(struct ev_loop *loop, ev_prepare *w, int revents);

void mfptp_idle_cb(struct ev_loop *loop, ev_idle *w, int revents);

void mfptp_async_cb(struct ev_loop *loop, ev_async *w, int revents);

void mfptp_check_cb(struct ev_loop *loop, ev_check *w, int revents);

void mfptp_update_cb(struct ev_loop *loop, ev_timer *w, int revents);

void mfptp_signal_cb(struct ev_loop *loop, ev_signal *w, int revents);

void mfptp_fetch_cb(struct ev_loop *loop, ev_io *w, int revents);

void mfptp_weibo_fetch_cb(struct ev_loop *loop, ev_io *w, int revents);

void _mfptp_send_cb(struct ev_loop *loop, ev_io *w, int revents);

void log_signal_cb(struct ev_loop *loop, ev_signal *w, int revents);

