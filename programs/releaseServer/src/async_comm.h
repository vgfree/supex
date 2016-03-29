#pragma once

#include "jtt_body.h"
#include "jtt_package.h"

void async_user_init(BODY_USER *user);

void async_user_reset(BODY_USER *user);

int async_user_is_online(BODY_USER user);

int async_user_connect(PKG_USER *p_user, BODY_USER *b_user);

