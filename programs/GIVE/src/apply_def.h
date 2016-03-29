#pragma once

#include "sniff_api.h"

#define OVERLOOK_DELAY_LIMIT 3

enum
{
	LIMIT_CHANNEL_KIND = 1,
};

struct mount_info
{
	SNIFF_WORKER_PTHREAD    *list;
	struct mount_info       *next;
};

