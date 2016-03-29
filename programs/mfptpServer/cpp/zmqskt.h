#ifndef __ZMQSKT_H__
#define __ZMQSKT_H__

// #define FALSE (1)
// #define TRUE  (0)
#include "basic_type.h"

typedef struct
{
	char    *front_addr;
	char    *back_addr;
} broker_para;

#define zmq_print_error()			       \
	printf("%s[%d]: %d: %s\n", __FILE__, __LINE__, \
		zmq_errno(), zmq_strerror(zmq_errno()));

/* 名  称: init_push_socket
 * 功  能: push -pull 模型中的push端SOCKET建立
 * 参  数:
 * 返回值:  TRUE表示成功，FALSE表示失败
 * 修  改: 新生成函数l00167671 at 2015/2/28
 */
void *mfptp_init_push_socket(char *end_point);

/* 名  称: mfptp_zmq_push_frame
 * 功  能: push -pull 模型中发送一个frame 给pull 端
 * 参  数:
 * 返回值: TRUE 表示成功，FALSE 表示失败
 * 修  改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_zmq_push_frame(void *skt, char *buf, int len, int more);

/* 名  称: start_proxy
 * 功  能: push -pull 模型的PROXY
 * 参  数:
 * 返回值: TRUE 表示成功，FALSE 表示失败
 * 修  改: 新生成函数l00167671 at 2015/2/28
 */
int start_proxy(void *arg);
#endif	/*__ZMQSKT_H__*/

