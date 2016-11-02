#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <hiredis.h>
#include "x_utils.h"

int set_user_online(char *id, char *redis_address, int redis_port);

int set_user_offline(char *id, char *redis_address, int redis_port);

