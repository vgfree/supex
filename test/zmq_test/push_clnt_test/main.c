//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/9/15.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <zmq.h>
#include "libmini.h"
#include "cJSON.h"

#define MAX_LEN_STRING 2048

struct pkgdt
{
	long    sec;
	int     microsec;
	char    seq[20];
};

bool checkzmqconnect(void *skt)
{
	int     opt = 0;
	int     flag = 0;
	size_t  vlen = sizeof(opt);

	assert(skt);

	x_printf(D, "test connection that whether has been completed or not.");

	flag = zmq_getsockopt(skt, ZMQ_EVENTS, &opt, &vlen);
	RAISE_SYS_ERROR(flag);

	if (opt & ZMQ_POLLOUT) {
		x_printf(D, "a message may be send on this socket.");
		return true;
	}

	return false;
}

void testzmqsndhwm(void *skt)
{
	int     opt = 3;
	int     flag = 0;
	int     vlen = sizeof(opt);

	assert(skt);

	x_printf(D, "set the hight water mark of sender to `%d`.", opt);

	flag = zmq_setsockopt(skt, ZMQ_SNDHWM, &opt, vlen);
	RAISE_SYS_ERROR(flag);
}

void setzmqimmediate(void *skt)
{
	int     flag = 0;
	int     opt = 1;
	size_t  vsize = sizeof(opt);

	flag = zmq_setsockopt(skt, ZMQ_IMMEDIATE, &opt, vsize);
	RAISE_SYS_ERROR(flag);
}

char TEST_HTTP_FORMAT[] = "POST /publicentry HTTP/1.1\r\n"
	"User-Agent: curl/7.33.0\r\n"
	"Host: %s:%d\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:%s\r\n"
	"Content-Length:%d\r\n"
	"Accept: */*\r\n\r\n"
	"%s";

int main(int argc, char **argv)
{
	volatile void   *zmqctx = NULL;
	volatile void   *zmqskt = NULL;

	TRY
	{
		char    tcpaddr[32] = {};
		int     flag = 0;

		SLogOpen(argv[0], NULL);

		zmqctx = zmq_ctx_new();
		AssertError(zmqctx, ENOMEM);

		if (unlikely(argc != 4)) {
#if 0
			x_printf(E, "Usage %s <host> <#port> <message>.", argv[0]);
#else
			x_printf(E, "Usage %s <host> <#port> <#loops>.", argv[0]);
#endif
			RAISE(EXCEPT_ASSERT);
		}

		zmqskt = zmq_socket((void *)zmqctx, ZMQ_PUSH);
		AssertRaise(zmqskt, EXCEPT_SYS);

#if 0
		setzmqimmediate(zmqskt);
#else
		testzmqsndhwm((void *)zmqskt);
#endif

		snprintf(tcpaddr, sizeof(tcpaddr), "tcp://%s:%d", argv[1], atoi(argv[2]));

		x_printf(D, "starting connect to %s.", tcpaddr);

		flag = zmq_connect((void *)zmqskt, tcpaddr);
		RAISE_SYS_ERROR(flag);

		while (!checkzmqconnect((void *)zmqskt)) {
			sleep(1);
		}

		x_printf(D, "connect successful and start send message.");

		long    sec;
		int     microsec;
		int     loops = atoi(argv[3]);

		const char poweron[] = "{\"powerOn\":true,\"accountID\":\"850212090004492\","
			"\"IMEI\":\"850212090004492\",\"tokenCode\":\"1adada912939\",\"model\":\"SG900\",\"srchost_time\":\"%ld.%d\"}";
		const char gps[] = "{\"collect\":true, \"accountID\":\"zdfeqE74Vi\", "
			"\"longitude\":[121.3977898,121.3977898,121.3977898,121.3977898,121.3977898], "
			"\"latitude\":[31.2202088,31.2202088,31.2202088,31.2202088,31.2202088], "
			"\"direction\":[150,150,150,150,150],\"speed\":[80,80,80,80,80],\"altitude\":[31,31,31,31,31],"
			"\"GPSTime\":[1231221,1231221,1231221,1231221,1231221],\"srchost_time\":\"%ld.%d\"}";

		int     bytes = 0;
		char    buff[MAX_LEN_STRING] = {};
		char    buff_poweron[1024] = {};
		char    buff_gps[1024] = {};

		do {
			TM_GetTimeStamp(&sec, &microsec);

			time_t  sec = 0;
			int     us = 0;

			/* get cJSON */
			char    *out = NULL;
			cJSON   *src = NULL;
			cJSON   *tag = NULL;

			src = cJSON_CreateObject();
			assert(src);

			tag = cJSON_CreateArray();
			assert(tag);
			cJSON_AddStringToObject(tag, "accountID", "850212090004492");
			cJSON_AddStringToObject(tag, "IMEI", "850212090004492");
			cJSON_AddItemToObject(src, "power", tag);
			out = cJSON_PrintUnformatted(src);

			/* get http body */
			bytes = zmq_send((void *)zmqskt, out, strlen(out), ZMQ_SNDMORE);
			RAISE_SYS_ERROR(bytes);

			TM_GetTimeStamp(&sec, &us);
			snprintf(buff_poweron, sizeof(buff_poweron), poweron, sec, us);

			bytes = zmq_send((void *)zmqskt, buff_poweron, strlen(buff_poweron), 0);
			RAISE_SYS_ERROR(bytes);

			free(out);
			cJSON_Delete(src);

			x_printf(D, "%s.", buff);

			src = cJSON_CreateObject();
			assert(src);

			tag = cJSON_CreateArray();
			assert(tag);
			cJSON_AddStringToObject(tag, "accountID", "zdfeqE74Vi");
			cJSON_AddStringToObject(tag, "grid", "121.3977898&31.2202088");
			cJSON_AddItemToObject(src, "gps", tag);

			out = cJSON_PrintUnformatted(src);

			/* get http body */

			bytes = zmq_send((void *)zmqskt, out, strlen(out), ZMQ_SNDMORE);
			RAISE_SYS_ERROR(bytes);

			TM_GetTimeStamp(&sec, &us);
			snprintf(buff_gps, sizeof(buff_gps), gps, sec, us);

			bytes = zmq_send((void *)zmqskt, buff_gps, strlen(buff_gps), 0);
			RAISE_SYS_ERROR(bytes);

			free(out);
			cJSON_Delete(src);

			x_printf(D, "%s.", buff);

			// usleep(100000);
		} while (--loops > 0);
	}
	CATCH
	{}
	FINALLY
	{
		int flag = 0;

		if (likely(zmqskt)) {
			zmq_close((void *)zmqskt);
		}

		while (zmqctx) {
			flag = zmq_ctx_term((void *)zmqctx);

			if (unlikely(flag < 0)) {
				if (likely(errno) == EINTR) {
					continue;
				}

				RAISE(EXCEPT_SYS);
			}

			break;
		}
	}
	END;

	return EXIT_SUCCESS;
}

