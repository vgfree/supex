#include "communication.h"
#include "router.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

char route_sign[7] = {0x52, 0x4f, 0x55, 0x54, 0x49, 0x0d, 0x0a}; /* route\r\n*/
