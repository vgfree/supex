#ifndef _MESSAGE_CONCENTRATOR_
#define _MESSAGE_CONCENTRATOR_

// 消息有2个来源，1, CoreExchangeNode mfptp格式，2, business server 的json 格式(c + lua 传统框架)。
// 消息有2个去处， 1, zmq 传送到业务服务器, 2, 重新组装mfptp 给CoreExchangeNode.
int message_fountain();

int concentrator_init();
void concentrator_destroy();
int router_msg();
int switch_msg();
#endif
