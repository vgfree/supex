//  简单聊天程序

#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

int main(void)
{
	/* 服务端流程
	 * 1. 创建一个新的ZMQ环境上下文
	 * 2. 创建ZMQ套接字
	 * 3. 绑定一个socket
	 * 4. 接受消息帧
	 * 5. 发送消息帧
	 */
	//  Socket to talk to clients
	void *context = zmq_ctx_new();
	// 创建一个ZMQ套接字 socket 类型为 ZMQ_REP
	void *responder = zmq_socket(context, ZMQ_REP);
	// 绑定一个socket
	int rc = zmq_bind(responder, "tcp://*:5555");

	// assert (rc == 0);

	while (1) {
		char buffer [128];
		// 接受一个消息帧
		zmq_recv(responder, buffer, 128, 0);
		printf("Miku : %s\n", buffer);
		//                sleep (1);          //  Do some 'work'
		// 回复一个消息帧
		char buffer1[128];
		fgets(buffer1, 128, stdin);
		printf("Shana : %s\n", buffer1);
		zmq_send(responder, buffer1, 128, 0);
	}

	return 0;
}

