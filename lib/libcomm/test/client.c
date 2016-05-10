#include "../communication.h"

#define		EPOLLSIZE	1024

static bool send_data(struct comm_context *commctx, struct comm_message *message, int fd);

static bool recv_data(struct comm_context *commctx, struct comm_message *message, int fd);

void event_fun(struct comm_context* commctx, struct portinfo *portinfo, void* usr);

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
					//log("client send_data successed\n");
				} else {
					log("client send_data failed\n");
				}

				memset(&recvmsg, 0, sizeof(recvmsg));
				retval = recv_data(commctx,&recvmsg, sendmsg.fd);
				if (likely(retval > 0)) {
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
	char buff[1024] = "nothingisimportant";
	int frames_of_package[PACKAGES] = {1,1,1};				/* 每个包里面帧数 */
	
	int frame_offset[FRAMES] = {0, 
					strlen("nothing"), 
					strlen("nothingis")
				};						/* 每帧的偏移 */
	int frame_size[FRAMES] = {	strlen("nothing"), 
					strlen("is"), 
					strlen("important")
				};						/* 每帧的大小 */

	message->fd = fd;
	message->content = buff;
	message->config = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->socket_type = REQ_METHOD;
	message->package.dsize = strlen(buff);
	message->package.frames = FRAMES;
	message->package.packages = PACKAGES;

	for (i = 0 ; i < PACKAGES; i++) {
		for (j = 0; j < frames_of_package[i]; j++, k++) {
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
	message->content = content;
	retval = comm_recv(commctx, message, true, 5000);
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
	message->fd = -1;
	printf("client here is close_fun()\n");
}

void write_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("client here is write_fun(): %s\n", message->content);
}

void read_fun(void *usr)
{
	struct comm_message *message = (struct comm_message*)usr;
	printf("client here is read_fun(): %s\n", message->content);
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
void event_fun(struct comm_context* commctx, struct portinfo *portinfo, void* usr)
{
	//log("client %d fd have action\n", portinfo->fd);
	switch (portinfo->stat)
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
			if (portinfo->type == COMM_ACCEPT) {
				accept_fun(usr); /* 当fd的类型为COMM_ACCEPT时才是accept事件 */
			}
		default:
			timeout_fun(usr);
			break;
	}
}
