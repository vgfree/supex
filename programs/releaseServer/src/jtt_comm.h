#pragma once

#include "jtt_body.h"
#include "jtt_package.h"
#include "jtt_client.h"

#define GPS_PKG_LEN (1024 * 5)		/*单个数据包的最大长度*/

int jtt_real_send(PKG_USER *pkg_user, BODY_USER body_user, BODY_GPS *gps, int n);

int jtt_extra_send(PKG_USER *pkg_user, BODY_USER body_user, BODY_GPS *extra, int n);

int jtt_heart(PKG_USER *pkg_user, BODY_USER *body_user);

// int jtt_logon(PKG_USER pkg_user,BODY_USER *body_user);

