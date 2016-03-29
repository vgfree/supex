/*
 * test_server.c
 *
 *  Created on: 2015/10/20
 *      Author: liubaoan@mirrtalk.com
 *
 *  Function:
 *      1.Test netmod.h API functions.
 */

#include "netmod.h"
#include <assert.h>
#include <libgen.h>
#include <stdio.h>

const char *sqltext[10] = {
	"INSERT INTO tab VALUES ('sdfsdf', 23, 'sdfsdfsdf', 2012);",
	"UPDATE tab SET name = 'JACK' WHERE age = 23;",
	"DELETE FROM tab where year = 2012;",
	"INSERT INTO tab VALUES ('sdfsdf', 24, 'sdfsdfsdf', 2013);",
	"UPDATE tab SET name = 'BLACK' WHERE age = 24;",
	"DELETE FROM tab where year = 2013;",
	"INSERT INTO tab VALUES ('sdfsdf', 25, 'sdfsdfsdf', 2014);",
	"UPDATE tab SET name = 'BLUCE' WHERE age = 25;",
	"DELETE FROM tab where year = 2014;",
	"INSERT INTO tab VALUES ('sdfsdf', 26, 'sdfsdfsdf', 2015);"
};

typedef struct
{
	char    name[63];
	char    gender;
	int     age;
	char    addr[128];
	char    tel[12];
	double  money;
} info_t;

enum dt_type { EV_STR, EV_STRUCT, EV_DOUBLE, EV_INT };

static event_t *assemble_event(int type, const event_t *ev);

static void *thrd_work(void *arg);

int main(int argc, char *argv[])
{
	int             error = 0;
	event_ctx_t     *ectx = event_ctx_init(&error, SOCK_SERVER, "tcp://*:8686", "");

	if (!ectx) {
		printf("%s\n", event_error(error));
		return -1;
	}

	pthread_t thrd;
	pthread_create(&thrd, NULL, thrd_work, ectx);

	while (1) {
		sleep(8);
	}

	pthread_join(thrd, NULL);

	event_ctx_destroy(ectx);

	return 0;
}

void *thrd_work(void *arg)
{
	assert(arg != NULL);

	event_ctx_t *ectx = arg;

	int     res, len, error = 0;
	event_t *ev, *ev_des;

	while (1) {
		ev = recv_event(&error, ectx, 20);

		if (!ev) {
			if (error != 0) {
				printf("recv_event: fail. Error - %s\n", event_error(error));
			}

			continue;
		}

		printf("Receive Client ID:[%s]\n", ev->id);
		// Handle Client requests.
		switch (ev->ev_type)
		{
			case EV_DUMP_REQ:
				break;

			case EV_INCREMENT_REQ:
				switch (ev->incr.task_seq)
				{
					case 0:	// send char *type.
						ev_des = assemble_event(EV_STR, ev);
						break;

					case 1:	// send struct info_t type.
						ev_des = assemble_event(EV_STRUCT, ev);
						break;

					case 2:	// send double type.
						ev_des = assemble_event(EV_DOUBLE, ev);
						break;

					case 3:	// send int type.
						ev_des = assemble_event(EV_INT, ev);
						break;

					default:
						ev_des = (event_t *)_alloc(sizeof(event_t));
						memcpy(ev_des, ev, sizeof(event_t));
						break;
				}
				break;

			default:
				break;
		}
		res = send_event(ectx, ev_des);

		if (res != 0) {
			printf("%s\n", event_error(res));
		}

		_free(ev_des);
	}	/* end while. */
}

event_t *assemble_event(int type, const event_t *ev)
{
	event_t *ev_des;

	int     len, year = 2015;
	double  money = 862400232342.98L;
	info_t  info = { "Richard.Liu", 'M', 23, "HongKong.", "021-2987653", money };

	switch (type)
	{
		case EV_STR:
			len = strlen(sqltext[0]) + 1;
			ev_des = (event_t *)_alloc(sizeof(event_t) + len);
			memcpy(ev_des->ev_data, sqltext[0], len);
			break;

		case EV_STRUCT:
			len = sizeof(info_t);
			ev_des = (event_t *)_alloc(sizeof(event_t) + len);
			memcpy(ev_des->ev_data, &info, len);
			break;

		case EV_DOUBLE:
			len = sizeof(double);
			ev_des = (event_t *)_alloc(sizeof(event_t) + len);
			memcpy(ev_des->ev_data, &money, len);
			break;

		case EV_INT:
			len = sizeof(int);
			ev_des = (event_t *)_alloc(sizeof(event_t) + len);
			memcpy(ev_des->ev_data, &year, len);
			break;

		default:
			break;
	}
	strcpy(ev_des->id, ev->id);
	ev_des->ev_type = EV_INCREMENT_REP;
	ev_des->ev_state = EV_NONE;
	ev_des->incr.task_seq = ev->incr.task_seq;
	ev_des->incr.rows = ev->incr.rows;
	ev_des->ev_size = len;

	return ev_des;
}

