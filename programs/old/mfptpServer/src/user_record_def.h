#pragma once

#include "rbtree.h"
#include <stdint.h>
#include "basic_type.h"
typedef struct user_record
{
	rb_node                 user_node;
	rb_node                 channel_node;
	uint64_t                user_id;
	void                    *user_data;
	char                    channel[15];
	struct user_record      *user_prev_record;
	struct user_record      *user_next_record;
	struct user_record      *channel_prev_record;
	struct user_record      *channel_next_record;
} USER_RECORD;

typedef USER_RECORD CHANNEL_RECORD;

