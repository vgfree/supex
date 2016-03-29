//  简单聊天程序
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	/* 客户端流程
	 * 1. 创建ZMQ环境上下文
	 * 2. 创建ZMQ套接字
	 * 3. 创建对外链接
	 * 4. 发送数据帧
	 * 5. 接收数据帧
	 * 6. 关闭套接字
	 * 7. 销毁ZMQ上下文
	 */
	printf("Connecting to hello world server…\n");
	// 创建一个新的ZMQ环境上下文
	void *context = zmq_ctx_new();
	// 创建ZMQ套接字
	void *requester = zmq_socket(context, ZMQ_REQ);
	// 由一个SOCKET创建一个对外连接
	zmq_connect(requester, "tcp://localhost:5555");

	char    buffer [128];
	char    buffer1 [128];
	int     request_nbr;

	// 循环向server端发送数据
	while (1) {
		fgets(buffer, 128, stdin);
		printf("Miku : %s\n", buffer);
		// 在一个socket上发送一个消息帧
		zmq_send(requester, buffer, 128, 0);
		// 在一个socket上接受一个消息帧
		zmq_recv(requester, buffer1, 128, 0);
		printf("Shana : %s\n", buffer1);
	}

	// 关闭ZMQ socket
	zmq_close(requester);
	// 销毁一个ZMQ环境上下文
	zmq_ctx_destroy(context);
	return 0;
}

