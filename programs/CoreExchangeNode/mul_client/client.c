#include "comm_message_operator.h"
#include "sys/time.h"
#include "communication.h"  

void print_current_time() {
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	printf("tv_sec:%d", tv.tv_sec);
	printf("tv_usec:%d--", tv.tv_usec);
}

static int send_data(struct comm_context *commctx, struct comm_message *message,
		int fd) {
	int i, ret = 0;
	int datasize = 1024;
	char *buff = (char *)malloc(datasize);
	memset(buff, 0, datasize);
	for(i = 0; i < datasize; i++) {
		buff[i] = '@';
	}	
	init_msg(message);
	set_msg_fd(message, fd);
	set_msg_frame(0, message, strlen(buff), buff); 
	printf("data:%s, data size:%d\n", message->content, message->package.dsize);
	ret = comm_send(commctx, message, false, -1);
	free(buff);
	return ret;
}

int main(int argc, char *argv[]) {
	int fd[10000] = {};
	int i;
	int datasize = 1024*1024;
	char *content = (char *)malloc(datasize);
	char *buff = (char *)malloc(datasize);

	struct comm_context  *commctx = NULL;
	struct comm_message  sendmsg  = {};
	struct comm_message  recvmsg  = {};
	struct cbinfo        cb = {};

	if(argc < 3) {
		printf("usge:%s <ip> <port> <client_count>\n", argv[0]);
		return -1;
	}
	commctx = comm_ctx_create(EPOLL_SIZE);
	printf("%s", commctx);
	if(!commctx) {
		printf("client context create failed\n");
		return -1;
	}
	recvmsg.content = content;
	for(i = 0; i < atoi(argv[3]); i++) {
		fd[i] = comm_socket(commctx, argv[1], argv[2], &cb, COMM_CONNECT);
		if(fd[i] == -1) {
			printf("establish connection failed\n");
			comm_ctx_destroy(commctx);
			return -1;
		}
		printf("client[%d]-fd[%d] establish connection successful!\n", i+1, fd[i]);

	}	

	while(1) {
		for(i = 0; i < atoi(argv[3]); i++) {
			if(send_data(commctx, &sendmsg, fd[i]) != -1) {
 				print_current_time();
				printf("client[%d]-fd[%d]:send data successfully\n", i+1,fd[i]);
				if(comm_recv(commctx, &recvmsg, true, -1) != -1) {
					printf("stay in here\n");
					size_t size;
					char    *frame = get_msg_frame(0, &recvmsg, &size); 
					buff = memcpy(buff, frame, size);
					buff[size] = '\0';
 					print_current_time();
				//	printf("recv_data: %s", buff);
				//	printf("recv_data_len: %d\n", strlen(buff));
					printf("recv_data:%*.s\n", recvmsg.package.dsize, recvmsg.content);
					free(buff);
				}else {
					printf("recv data failed\n");
				}
			}else {
				printf("send data failed\n");
			} 
		}
	}
	destroy_msg(&sendmsg);
	destroy_msg(&recvmsg);
	comm_ctx_destroy(commctx);
	return 0;
}
