#include "../communication.h"

#define		EPOLLSIZE	1024

static bool send_data(struct comm_context *commctx, struct comm_message *message, int fd);

static bool recv_data(struct comm_context *commctx, struct comm_message *message, int fd);

void event_fun(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr);

int main(int argc, char* argv[])
{
	int i = 0, fd = -1;
	int retval = -1;
	struct cbinfo		finishedcb = {0};
	struct comm_context*	commctx = NULL;
	struct comm_message	sendmsg = {0};
	struct comm_message	recvmsg = {0};

	if( unlikely(argc < 3) ){
		printf("usage: %s <ipaddr> <port>", argv[0]);
		return retval;
	}

	commctx = comm_ctx_create(EPOLLSIZE);
	if (likely(commctx)) {
		log("server comm_ctx_create successed\n");
		finishedcb.callback = event_fun;
		finishedcb.usr = &recvmsg;
		fd = comm_socket(commctx, argv[1], argv[2], &finishedcb, COMM_BIND);
		if (likely(fd > 0)) {
			log("server comm_socket successed\n");
			while (1) {
				memset(&recvmsg, 0, sizeof(recvmsg));
				retval = recv_data(commctx, &recvmsg, -1);
				if (likely(retval > 0)) {
					log("server recv_data successed\n");
				} else {
				//	log("server recv_data failed\n");
				//	sleep(1);
				}
				if (recvmsg.fd > 0) {
					memset(&sendmsg, 0, sizeof(sendmsg));
					retval = send_data(commctx, &sendmsg, recvmsg.fd);
					if (likely(retval > 0)) {
						log("server send_data successed\n");
						//break ;
					} else {
					//	log("server send_data failed\n");
					}
				}
			}
		} else {
			log("server comm_socket failed\n");
		}
#if 0
		comm_close(commctx, fd);
		fd = comm_socket(commctx, "127.0.0.1", "10004", &finishedcb, COMM_BIND);
		if (likely(fd > 0)) {
			log("server comm_socket successed\n");
			while (1) {
				memset(&recvmsg, 0, sizeof(recvmsg));
				retval = recv_data(commctx, &recvmsg, -1);
				if (likely(retval > 0)) {
					//log("server recv_data successed\n");
				} else {
					log("server recv_data failed\n");
					sleep(1);
				}
				if (recvmsg.fd > 0) {
					memset(&sendmsg, 0, sizeof(sendmsg));
					retval = send_data(commctx, &sendmsg, recvmsg.fd);
					if (likely(retval > 0)) {
						//log("server send_data successed\n");
						break ;
					} else {
						log("server send_data failed\n");
					}
				}
			}
		} else {
			log("server comm_socket failed\n");
		}
#endif
	}

	comm_ctx_destroy(commctx);
	return retval;
}

static bool send_data(struct comm_context *commctx, struct comm_message *message, int fd)
{
#undef FRAMES
#define FRAMES	3
#undef PACKAGES 
#define PACKAGES 3

	int i = 0, j = 0, k = 0;
	int retval = -1; 
	char buff[1024] = "everythingisimportant";
	int frames_of_package[PACKAGES] = {1,1,1};				/* 每个包里面帧数 */
	
	int frame_offset[FRAMES] = {0, 
					strlen("everything"), 
					strlen("everythingis")
				};						/* 每帧的偏移 */
	int frame_size[FRAMES] = {	strlen("everything"), 
					strlen("is"), 
					strlen("important")
				};						/* 每帧的大小 */

	message->fd = fd;
	message->content = buff;
	message->config = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->socket_type = REP_METHOD;
	message->package.dsize = strlen(buff);
	message->package.frames = FRAMES;
	message->package.packages = PACKAGES;

	for (i = 0 ; i < PACKAGES; i++) {
		for (j = 0; j < frames_of_package[i]; j++,k++) {
			message->package.frame_offset[k] = frame_offset[k];
			message->package.frame_size[k] = frame_size[k];
		}
	} 
	memcpy(message->package.frames_of_package, frames_of_package, (sizeof(int))*PACKAGES);

	retval = comm_send(commctx, message, false, -1);
	if (retval <=  0) {
		return false;
	} else {
		return true;
	}
}

static bool recv_data(struct comm_context *commctx, struct comm_message *message, int fd) 
{
	char buff[1024] = {0};
	char content[1024] = {0};
	int retval = -1;
	int i = 0, j = 0, k = 0; 
	int size = 0;
	message->content = content;
	retval = comm_recv(commctx, message, true, -1);
	if( unlikely(retval < 0) ){
		log("comm_recv failed\n");
		return false;
	} else {
		for (i = 0; i < message->package.packages; i++) {
			size = 0;
			memset(buff, 0, 1024);
			for (j = 0; j < message->package.frames_of_package[i]; j++, k++) {
				memcpy(&buff[size], &message->content[message->package.frame_offset[k]], message->package.frame_size[k]);
				size += message->package.frame_size[k];
				buff[size++] = ' ';
			}
			log("%s\n", buff);

		}
		log("comm_recv successed\n");
		return true;
	}
}

void close_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("server here is close_fun():%d\n", message->fd);
	message->fd = -1;
}

void write_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("server here is write_fun(): %d\n", message->fd);
}

void read_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("server here is read_fun(): %d\n", message->fd);
}

void accept_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("server here is accept_fun(): %d\n", commtcp->fd);
}

void timeout_fun(void *usr)
{
	printf("server here is timeout_fun()\n");
}

void event_fun(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr)
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
				accept_fun(commtcp);
			}
		default:
			timeout_fun(usr);
			break;
	}
}
