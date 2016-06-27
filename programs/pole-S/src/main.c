#include "netmod.h"
#include "clog.h"
#include "parser.h"
#include "register.h"
#include "busi_db.h"
#include "busi_dump.h"
#include "busi_w2file.h"

static int send_result(const evt_ctx_t *ev_ctx, const evt_t *ev, int res, char *error);

int main(int argc, char *argv[])
{
	int             error;
	int             res, len;
	sync_conf_t     confs;
	evt_ctx_t       *ev_ctx;
	evt_t         ev_des, *ev;

	/* Parsing configure file. */
	res = parse_config(&confs, "./pole-S_conf.json");
	assert(res == 0);

	/* Initialize the Pole-S Log file. */
	res = log_ctx_init(confs.log_file_path, confs.log_level);
	assert(res == 0);

	/* Initialize the Network. */
	ev_ctx = evt_ctx_init(&error, SOCK_CLIENT, confs.conn_uri, confs.id);

	if (ev_ctx == NULL) {
		x_printf(E, "evt_ctx_init(,,'%s','%s') fail. Error - %s\n", confs.conn_uri, confs.id, evt_error(error));
		return -1;
	}

	// Startup the thread of Network Center to Recv & Send data.
	pthread_t       thrd;
	pthread_attr_t  attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int ok = pthread_create(&thrd, &attr, thrd_netCenter, ev_ctx);
	return_if_false(ok == 0, NULL);

	/* Register the business functions. */
	business_t busi_dump = { dump_init, dump_db, NULL };

	// busi_incr you can modify it's elements with your own.
	// But busi_dump, you should never modify it.
	business_t busi_incr = {
		w2file_init,
		w2file_done,
		w2file_destroy
	};

	register_business(BUSI_DUMP, &busi_dump);
	register_business(BUSI_INCR, &busi_incr);

	/* Startup all the businesses. */
	res = startup_businesses(&confs);
	assert(res == 0);

	/* Execute by current running type. */
	if (!strcmp(confs.run_type, "EV_DUMP")) {
		/* Current Running type is Synchronizing the database base data. */

		/* Send NET_EV_DUMP_REQ command to Server. */
		len = strlen(confs.dump_id) + 1;

		if (len == 1) {
			x_printf(W, "Warning: The pole-S_conf.json file DES_DUMP_ID should be set, when event type is EV_DUMP.\n");
			goto FAIL;
		}

		evt_t *ev_dump = evt_new_by_size(len);

		if (!ev_dump) {
			x_printf(E, "evt_new_by_size(evt_t:%d) bytes fail. Error - %s.\n", len, strerror(errno));
			goto FAIL;
		}

		strcpy(ev_dump->id, confs.id);
		ev_dump->ev_type = NET_EV_DUMP_REQ;
		ev_dump->ev_state = NET_EV_NONE;
		// ev_dump->ev_size = len; // ev_size was set by evt_new_by_size().
		strcpy(ev_dump->ev_data, confs.dump_id);

		res = send_evt(ev_ctx, ev_dump);

		if (res != 0) {
			x_printf(E, "send_evt() fail. Error - %s\n", evt_error(errno));
			goto FAIL;
		}

		print_evt(ev_dump);

		free_evt(ev_dump);

		/* Recv NET_EV_DUMP_REP command from Server. */
		/* We'll wait for a long time, Because Dump DB is very consume time. */
		ev = recv_evt(ev_ctx, -1);
		print_evt(ev);

		if (ev->ev_state == NET_EV_FAIL) {
			x_printf(E, "Destination Host, dump the MySQL database data fail.\n");
			goto FAIL;
		}

		x_printf(I, "Destination Host, has already dumped the MySQL database succeed.\n");
		free_evt(ev);

		/* Process exit. */
		evt_ctx_destroy(ev_ctx);
		return 0;
	} else if (!strcmp(confs.run_type, "EV_INCREMENT")) {
		/* Current Running type is Synchronizing the increment data. */
		char errinfo[128];

		/* Send NET_EV_INCREMENT_REQ command to Server, for the first time. */
		strcpy(ev_des.id, confs.id);
		ev_des.ev_type = NET_EV_INCREMENT_REQ;
		ev_des.ev_state = NET_EV_NONE;
		ev_des.ev_size = 0;
		ev_des.incr.task_seq = 0;	// First time, we don't know the task_seq.
		ev_des.incr.rows = 1;

		res = send_evt(ev_ctx, &ev_des);

		if (res != 0) {
			x_printf(E, "send_evt() fail. Error - %s\n", evt_error(res));
			goto FAIL;
		}

		print_evt(&ev_des);

		while (1) {
			/* Recv Server's response. */
			ev = recv_evt(ev_ctx, -1);
			print_evt(ev);

			/* Doing businesses. return NET_EV_SUCC|NET_EV_FAIL|NET_EV_FATAL. */
			res = do_business(ev->ev_type, errinfo, sizeof(errinfo), (ev->ev_size > 0) ? ev->ev_data : NULL, ev->ev_size);

			/* Send Client Business execute state. */
			res = send_result(ev_ctx, ev, res, errinfo);

			if (res != 0) {
				x_printf(E, "send_result() fail. Error - %s.\n", evt_error(res));
				goto FAIL;
			}

			free_evt(ev);
		}
	} else {
		x_printf(E, "Invalid Running Type value <%s>\n", confs.run_type);
		return -1;
	}

	return 0;

FAIL:
	evt_ctx_destroy(ev_ctx);
	return -1;
}

int send_result(const evt_ctx_t *ev_ctx, const evt_t *ev, int res, char *errinfo)
{
	evt_t ev_des, *ev_err;

	switch (ev->ev_type)
	{
		case BUSI_DUMP:	// See the register.h Business type define.

			if (res == NET_EV_SUCC) {
				ev_err = evt_new_by_size(IDENTITY_SIZE);
				strcpy(ev_err->id, ev->id);
				ev_err->ev_type = NET_EV_DUMP_REP;
				ev_err->ev_state = res;	// res == NET_EV_FAIL|NET_EV_FATAL.
				ev_err->ev_size = strlen(ev->ev_data) + 1;
				strcpy(ev_err->ev_data, ev->ev_data);

				res = send_evt(ev_ctx, ev_err);
				x_printf(I, "DO_DUMP_REP: Send succeed, ID:<%s> DES:<%s.", ev_err->id, ev_err->ev_data);
				print_evt(ev_err);
				free_evt(ev_err);
			} else {
				ev_err = evt_new_by_size(strlen(errinfo) + 1);
				strcpy(ev_err->id, ev->id);
				ev_err->ev_type = NET_EV_DUMP_REP;
				ev_err->ev_state = res;	// res == NET_EV_FAIL|NET_EV_FATAL.
				strcpy(ev_err->ev_data, errinfo);

				res = send_evt(ev_ctx, ev_err);
				print_evt(ev_err);
				free_evt(ev_err);
			}

			break;

		case BUSI_INCR:

			if (res == NET_EV_SUCC) {
				strcpy(ev_des.id, ev->id);
				ev_des.ev_type = NET_EV_INCREMENT_REQ;
				ev_des.ev_state = NET_EV_SUCC;
				ev_des.ev_size = 0;
				ev_des.incr.rows = 1;
				ev_des.incr.task_seq = ev->incr.task_seq + 1;

				res = send_evt(ev_ctx, &ev_des);
				print_evt(&ev_des);
			} else {
				ev_err = evt_new_by_size(strlen(errinfo) + 1);
				strcpy(ev_err->id, ev->id);
				ev_err->ev_type = NET_EV_INCREMENT_REQ;
				ev_err->ev_state = res;
				ev_err->incr.rows = 1;
				ev_err->incr.task_seq = ev->incr.task_seq;
				strcpy(ev_err->ev_data, errinfo);

				res = send_evt(ev_ctx, ev_err);
				print_evt(ev_err);
				free_evt(ev_err);
			}

			break;

		default:
			x_printf(E, "Invalid event type.\n");
			break;
	}

	return res;
}

