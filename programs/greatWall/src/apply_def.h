#pragma once

#include "minor/sniff_api.h"

#define OVERLOOK_DELAY_LIMIT            3
#define RESEND_PROTECT_TIME_DELAY       2

enum
{
	REAL_TIME_KIND = 0,
	NON_REAL_TIME_KIND,
	LIMIT_CHANNEL_KIND,
};

#define MAX_SNIFF_LABEL_LENGTH  64
#define MAX_SNIFF_FLOWS_LENGTH  (MAX_SNIFF_DATA_SIZE - MAX_SNIFF_LABEL_LENGTH)

struct route_msg_data
{
	char    label[MAX_SNIFF_LABEL_LENGTH];
	char    flows[MAX_SNIFF_FLOWS_LENGTH];
};

