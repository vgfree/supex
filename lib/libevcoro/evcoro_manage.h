#pragma once

#include "evcoro_scheduler.h"

int evcoro_manage_push(struct evcoro_scheduler *esc);

int evcoro_manage_pull(struct evcoro_scheduler *esc);

void evcoro_manage_print(void);
