#pragma once

#include <string.h>
#include "rr_cfg.h"
//#include "async_api.h"
#include "ev.h"
#include "utils.h"

/*检查imei是否是测试imei*/
bool forward_imei_check(char* imei);

/*将数据转发到测试服务器上*/
bool forward_data(const char *data, struct ev_loop *loop, char *host, int port);

