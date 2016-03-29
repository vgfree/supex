#pragma once

#include "xmq.h"
#include "netmod.h"

extern int switcher_work(event_ctx_t *ev_ctx, xmq_ctx_t *xmq_ctx);

extern void switcher_join_all_porter(void);

