#include "daemon.h"
#include "fountain.h"
#include "loger.h"

#include <signal.h>

#define SERVER_FILE     "loginServer.pid"
#define MODULE_NAME     "loginServer"

struct CSLog *g_imlog = NULL;
int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);

	if (daemon_init(SERVER_FILE) == 1) {
		printf("server is running");
		return -1;
	}

	g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
	fountain_init();
	message_fountain();
	fountain_destroy();
	daemon_exit(SERVER_FILE);
	CSLog_destroy(g_imlog);
	return 0;
}

