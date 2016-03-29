#ifndef _SEND_DATA_H_
#define _SEND_DATA_H_

#include "cqueue.h"

typedef struct _dtpkt
{
	int     dt_size;
	char    dt_data[0];
} dtpkt_t;

int startSendThread(const char *server, int port);

int destroySendThread();

int pushData(const void *data, size_t size);

dtpkt_t *popData();
#endif

