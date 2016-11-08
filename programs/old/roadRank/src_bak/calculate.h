#ifndef __CALCULATE_H_
#define __CALCULATE_H_

#include <stdio.h>
#include <stdlib.h>
#include <ev.h>

#include "decode_gps.h"
#include "rr_cfg.h"

int calculate(GPS_INFO *gps_info, void *data, struct ev_loop *loop);
#endif

