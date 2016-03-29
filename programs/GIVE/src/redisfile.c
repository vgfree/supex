#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "redis_interface.h"

const char      *HOST = "192.168.71.61";
const int       PORT = 5072;

int main(int argc, char **argv)
{
	redisContext *con_status;

	printf("start.\n");
	connect_toredis(HOST, PORT, &con_status);
	loadhkeys_tofile(con_status, "Hkeys-ct");
	printf("done.\n");
	free_connect(con_status);
	return 0;
}

