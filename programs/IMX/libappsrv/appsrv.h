#ifndef _APP_SRV_H_
#define _APP_SRV_H_

#include "zmq.h"

#include <sys/uio.h>


enum askt_type {
	TYPE_UPSTREAM		= 0x00000001,
	TYPE_DOWNSTREAM		= 0x00000010,
	TYPE_STATUS		= 0x00000100,
	TYPE_SETTING		= 0x00001000,
	TYPE_LOOKING		= 0x00010000,
};

#define SRV_CONFIG                  "libappsrv.conf"

#define GATEWAY_PUSH_HOST		"conn_gateway_push_upstream_host"
#define GATEWAY_PUSH_PORT		"conn_gateway_push_upstream_port"


#define GATEWAY_PULL_HOST		"gateway_pull_downstream_host"
#define GATEWAY_PULL_PORT		"gateway_pull_downstream_port"

#define LOGIN_PUSH_HOST              	"conn_login_push_status_host"
#define LOGIN_PUSH_PORT			"conn_login_push_status_port"

#define USERINFOAPI_PULL_HOST		"usrinfoapi_pull_setting_host"
#define USERINFOAPI_PULL_PORT		"usrinfoapi_pull_setting_port"

#define USERINFOAPI_REP_HOST		"usrinfoapi_rep_looking_host"
#define USERINFOAPI_REP_PORT		"usrinfoapi_rep_looking_port"


#define MAX_SPILL_DEPTH    32

struct app_msg
{
	size_t          vector_size;
	struct iovec    vector[MAX_SPILL_DEPTH];
};

void create_io(int types);

void destroy_io(void);

int app_recv_msg(enum askt_type type, struct app_msg *msg);

int app_send_msg(enum askt_type type, struct app_msg *msg);
	
int app_recv_more_msg(int types, struct app_msg *msg, int flag);

#endif	/* ifndef _APP_SRV_H_ */

