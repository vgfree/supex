/*
 * test_client.c
 *
 *  Created on: 2015/10/20
 *      Author: liubaoan@mirrtalk.com
 */

#include "netmod.h"
#include <stdio.h>

typedef struct
{
	char    name[63];
	char    gender;
	int     age;
	char    addr[128];
	char    tel[12];
	double  money;
} info_t;

static void test_exec_cmd();

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Usage: ./client identity conn_uri\n");
		return -1;
	}

	int i, res, error = 0;
	printf("Client ID:[%s] CONN_URI:[%s]\n", argv[1], argv[2]);

	info_t  *info;
	double  money;
	int     year;
	event_t *ev, *ev_dump, ev_des;

	event_ctx_t *ectx = event_ctx_init(&error, SOCK_CLIENT, argv[2], argv[1]);

	if (ectx == NULL) {
		printf("Main: event_ctx_init() fail. Error - %s\n", event_error(error));
		return -1;
	}

	/* Test[0]: Test transfer different types of data. */
	for (i = 0; i <= 4; i++) {
		strcpy(ev_des.id, argv[1]);
		ev_des.ev_type = EV_INCREMENT_REQ;
		ev_des.ev_state = EV_NONE;
		ev_des.incr.task_seq = i;
		ev_des.incr.rows = 1;
		ev_des.ev_size = 0;

		res = send_event(ectx, &ev_des);

		if (res != 0) {
			printf("Test[0]: send_event(task_id:%d) fail. %s\n", i, event_error(res));
		}

RECV_AGAIN:
		ev = recv_event(&error, ectx, 10);

		if (!ev) {
			if (error != 0) {
				printf("recv_event: fail. Error - %s\n", event_error(error));
			}

			goto RECV_AGAIN;
		}

		switch (ev->incr.task_seq)
		{
			/* Test: Contain event-body type. */
			case 0:	// char* type. SQL request.
				printf("Test[0]: recv_event(char *): SQLText:[%s].\n", ev->ev_data);
				break;

			case 1:	// structure type. info_t.
				info = (info_t *)ev->ev_data;
				printf("Test[0]: recv_event(struct): info_t: {Name:%s Sex:'%c' Age:%d Addr:%s Tel:%s Money:%lf}\n", \
					info->name, info->gender, info->age, info->addr, info->tel, info->money);
				break;

			case 2:	// double type.
				money = *(double *)(ev->ev_data);
				printf("Test[0]: recv_event(double): money:%lf\n", money);
				break;

			case 3:
				year = *(int *)(ev->ev_data);
				printf("Test[0]: recv_event(int): year:%d\n", year);
				break;

			default:
				/* Test: Don't contain event-body type. */
				assert(ev_des.ev_type == ev->ev_type);
				assert(ev_des.ev_state == ev->ev_state);
				assert(ev_des.incr.task_seq == ev->incr.task_seq);
				assert(ev_des.incr.rows == ev->incr.rows);
				printf("Test[0]: recv_event(null-body): only the event-head.\n");
				break;
		}

		// if (i % 2 == 1) sleep(6);
	}

	event_ctx_destroy(ectx);

	return 0;
}

void test_exec_cmd()
{
	int     res, i;
	char    result[1024];
	char    *dump[] = {
		"mysqldump -hlocalhost -uhr -phr -A > basedump_h0.sql",
		"mysqldump -h192.168.11.27 -uhr -phr -A > basedump_h1.sql",
		"mysqldump -A > basedump_h2.sql",
		"ls -al",
		"echo Hello World.",
		NULL
	};

	for (i = 0; dump[i] != NULL; i++) {
		res = _exec_cmd(dump[i], result, sizeof(result));
		printf("Result[%d]:%s\n", i, result);
	}
}

/*
 * Print Result:
 *
 * Test[0]: recv_event(char *): SQLText:[INSERT INTO tab VALUES ('sdfsdf', 23, 'sdfsdfsdf', 2012);].
 * Test[0]: recv_event(struct): info_t: {Name:Richard.Liu Sex:'M' Age:23 Addr:HongKong. Tel:021-2987653 Money:862400232342.979980}
 * Test[0]: recv_event(double): money:862400232342.979980
 * Test[0]: recv_event(int): year:2015
 * Test[0]: recv_event(null-body): only the event-head.
 */

