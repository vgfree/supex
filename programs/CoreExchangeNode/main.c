#include "message_dispatch.h"
#include "loger.h"
#include "communication.h"
#include "fd_manager.h"

#include <stdlib.h>

void io_notify_logic_thread(struct commctx* commctx, int fd,
							struct message_notify *msg_notify)
{
	switch (msg_notify->cmd) {
		// TO DO.
	case APPEND:
	  append_router_object(msg_obj, fd);	
	}
}

int main(int argc, char* argv[])
{
    struct CSLog* g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
	if( unlikely(argc < 3) ){
		printf("usage: %s <ipaddr> <port>", argv[0]);
		return -1;
	}

	struct comm *commctx = NULL;
	commctx = comm_ctx_create(EPOLL_SIZE);
	if( unlikely(!commctx) ){
		return -1;
	}
	struct cbinfo  finishedcb = {};
	finishedcb.callback = io_notify_logic_thread;

	int retval = comm_socket(commctx, argv[1], argv[2], finishedcb, COMM_BIND);
	if(retval == -1){
		error("can't bind socket.");
		return retval;
	}

	while( 1 ){
		message_dispatch();
	}
	CSLog_free(g_imlog);
}
