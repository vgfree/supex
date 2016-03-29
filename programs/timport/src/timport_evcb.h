#pragma once

#include "ev.h"

void timport_signal_cb(struct ev_loop *loop, ev_signal *w, int revents);

void timport_timer_cb(struct ev_loop *loop, ev_timer *w, int revents);

