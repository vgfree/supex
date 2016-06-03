#include "../communication.h"

#define		EPOLLSIZE	1024

static bool send_data(struct comm_context *commctx, struct comm_message *message, int fd);

static bool recv_data(struct comm_context *commctx, struct comm_message *message, int fd);

void event_fun(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr);

int main(int argc, char* argv[])
{
	int i = 0, fd = -1;
	int retval = -1;
	struct comm_message	sendmsg = {0};
	struct comm_message	recvmsg = {0};
	struct comm_context	*commctx = NULL;
	struct cbinfo		finishedcb = {};

	if( unlikely(argc < 3) ){
		printf("usage: %s <ipaddr> <port>", argv[0]);
		return retval;
	}

	commctx = comm_ctx_create(EPOLLSIZE);
	if (likely(commctx)) {
		log("client comm_ctx_create successed\n");
		finishedcb.callback = event_fun;
		finishedcb.usr = &sendmsg;
		fd = comm_socket(commctx, argv[1], argv[2], &finishedcb, COMM_CONNECT);
		if (likely(fd > 0)) {
			log("client comm_socket successed\n");
			while (1) {
				memset(&sendmsg, 0, sizeof(sendmsg));
				retval = send_data(commctx, &sendmsg, fd);
				if (likely(retval > 0)) {
				//	log("client send_data successed\n");
				} else {
					//log("client send_data failed\n");
				}
				sleep(2);

				memset(&recvmsg, 0, sizeof(recvmsg));
				retval = recv_data(commctx,&recvmsg, sendmsg.fd);
				if (likely(retval > 0)) {
					log("client recv_data successed\n");
				//	break ;
				} else {
					//log("client recv_data failed\n");
				//	sleep(1);
				}
			}
		} else {
			log("client comm_socket failed\n");
		}
	} else {
		log("client comm_ctx_create failed\n");
	}

#if	0
	if (likely(commctx)) {
		finishedcb.callback = event_fun;
		finishedcb.usr = &sendmsg;
		fd = comm_socket(commctx, "127.0.0.1", "10004", &finishedcb, COMM_CONNECT);
		if (likely(fd > 0)) {
			log("client comm_socket successed\n");
			while (1) {
				memset(&sendmsg, 0, sizeof(sendmsg));
				retval = send_data(commctx, &sendmsg, fd);
				if (likely(retval > 0)) {
					//log("client send_data successed\n");
				} else {
					log("client send_data failed\n");
				}

				memset(&recvmsg, 0, sizeof(recvmsg));
				retval = recv_data(commctx,&recvmsg, sendmsg.fd);
				if (likely(retval > 0)) {
					break ;
				//	log("client recv_data successed\n");
				} else {
					log("client recv_data failed\n");
					sleep(1);
				}
			}
		} else {
			log("client comm_socket failed\n");
		}
	} else {
		log("client comm_ctx_create failed\n");
	}
#endif

	comm_ctx_destroy(commctx);
	return retval;
}

static bool send_data(struct comm_context *commctx, struct comm_message *message, int fd)
{
#undef FRAMES
#define FRAMES	3
#undef PACKAGES
#define PACKAGES 1

	int i = 0, j = 0, k = 0;
	int retval = -1; 
	char buff[1024] = "nothingisimportant";
	char buff1[1024] = "nothingmattersI'monmyown";
	int frames_of_package[1] = {3};				/* 每个包里面帧数 */
	int frames_of_package1[2] = {2, 4};
	
	int frame_offset[3] = {0, 
					strlen("nothing"), 
					strlen("nothingis")
				};						/* 每帧的偏移 */
	int frame_offset1[6] = {0,
					strlen("nothing"),
					strlen("nothingmatters"),
					strlen("nothingmattersI'm"),
					strlen("nothingmattersI'mon"),
					strlen("nothingmattersI'monmy")
				};
	int frame_size[3] = {	strlen("nothing"), 
					strlen("is"), 
					strlen("important")
				};						/* 每帧的大小 */
	int frame_size1[6] = {
					strlen("nothing"),
					strlen("matters"),
					strlen("I'm"),
					strlen("on"),
					strlen("my"),
					strlen("own")
				};

	message->fd = fd;
	message->content = buff;
	message->config = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->socket_type = REQ_METHOD;
	message->package.dsize = strlen(buff);
	message->package.frames = 3;
	message->package.packages = 1;

	for (i = 0 ; i < message->package.packages; i++) {
		for (j = 0; j < frames_of_package[i]; j++, k++) {
			message->package.frame_offset[k] = frame_offset[k];
			message->package.frame_size[k] = frame_size[k];
		}
	} 
	memcpy(message->package.frames_of_package, frames_of_package, (sizeof(int))*message->package.packages);

	retval = comm_send(commctx, message, false, -1);
	if (retval <=  0) {
		return false;
	} else {
		/* 进行第二次发送数据 */
		message->content = buff1;
		message->package.packages = 2;
		message->package.frames = 6;
		message->package.dsize = strlen(buff1);
		k = 0;
		for (i = 0 ; i < message->package.packages; i++) {
			for (j = 0; j < frames_of_package1[i]; j++, k++) {
				message->package.frame_offset[k] = frame_offset1[k];
				message->package.frame_size[k] = frame_size1[k];
			}
		} 
		memcpy(message->package.frames_of_package, frames_of_package1, (sizeof(int))*message->package.packages);
		retval = comm_send(commctx, message, false, -1);
		if (retval <= 0) {
			return false;
		} else {
			return true;
		}
	}
}

static bool recv_data(struct comm_context *commctx, struct comm_message *message, int fd) 
{
	char buff[1024] = {0};
	char content[1024] = {0};
	int retval = -1;
	int i = 0, j = 0, k = 0; 
	message->content = content;
	retval = comm_recv(commctx, message, false, 5000);
	if( unlikely(retval < 0) ){
		return false;
	} else {
		for (i = 0; i < message->package.packages; i++) {
			for (j = 0; j < message->package.frames_of_package[j]; j++, k++) {
				memset(buff, 0, 1024);
				memcpy(buff, &message->content[message->package.frame_offset[k]], message->package.frame_size[k]);
				log("%s\n", buff);
			}
			//log("one package over\n");
		}
		return true;
	}
}

void close_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("client here is close_fun():%d\n", message->fd);
	message->fd = -1;
}

void write_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("client here is write_fun(): %d\n", message->fd);
}

void read_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("client here is read_fun(): %d\n", message->fd);
}

void accept_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("client here is accept_fun(): %d\n", message->fd);
}

void timeout_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("client here is timeout_fun()\n");
}

/* @status: FD_CLOSE, FD_READ, FD_WRITE */
void event_fun(struct comm_context* commctx, struct comm_tcp *commtcp, void* usr)
{
	switch (commtcp->stat)
	{
		case FD_CLOSE:
			close_fun(usr);
			break;
		case FD_WRITE:
			write_fun(usr);
			break;
		case FD_READ:
			read_fun(usr);
			break;
		case FD_INIT:
			if (commtcp->type == COMM_ACCEPT) {
				accept_fun(usr); /* 当fd的类型为COMM_ACCEPT时才是accept事件 */
			}
		default:
			timeout_fun(usr);
			break;
	}
}
