#pragma once

#include "evcoro_scheduler.h"

ssize_t evcoro_dskio_pwrite(struct evcoro_scheduler *scheduler, int fd, const void *buf, size_t count, off_t offset);
