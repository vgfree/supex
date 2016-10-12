#pragma once

#include <stdbool.h>
bool sniff_task_report(void *user, void *task);

bool sniff_task_lookup(void *user, void *task);

void app_queue_init(void);

