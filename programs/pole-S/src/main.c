#include "netmod.h"
#include "conf.h"
#include "register.h"
#include "busi_db.h"
#include "busi_dump.h"
#include "busi_w2file.h"


int main(int argc, char *argv[])
{
	int             res;
	sync_conf_t     confs;
	evt_ctx_t       *evt_ctx;
	evt_t           ev_des, *evt;

	/* Parsing configure file. */
	res = parse_config(&confs, "./pole-S_conf.json");
	assert(res == 0);

	/* Initialize the Pole-S Log file. */
	SLogOpen(confs.log_file, SLogIntegerToLevel(confs.log_level));

	/* Initialize the Network. */
	evt_ctx = evt_ctx_init(SOCK_CLIENT, confs.conn_uri, confs.id);

	if (evt_ctx == NULL) {
		x_printf(E, "evt_ctx_init(,,'%s','%s') fail. Error\n", confs.conn_uri, confs.id);
		return -1;
	}

	// Startup the thread of Network Center to Recv & Send data.
	pthread_t       thrd;
	pthread_attr_t  attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int ok = pthread_create(&thrd, &attr, work_evt, evt_ctx);
	return_if_false(ok == 0, NULL);

	/* Register the business functions. */
	business_t busi_dump = {
		dump_init,
		dump_db,
		NULL
	};

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
		int len = strlen(confs.dump_id) + 1;

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

		print_evt(ev_dump);
		assert(0 == send_evt(evt_ctx, ev_dump));

		/* Recv NET_EV_DUMP_REP command from Server. */
		/* We'll wait for a long time, Because Dump DB is very consume time. */
		do {
			sleep(1);
			evt = recv_evt(evt_ctx);
		} while(!evt);

		print_evt(evt);

		if (evt->ev_state == NET_EV_FAIL) {
			x_printf(E, "Destination Host, dump the MySQL database data fail.\n");
			goto FAIL;
		}

		x_printf(I, "Destination Host, has already dumped the MySQL database succeed.\n");
		free_evt(evt);

		/* Process exit. */
		evt_ctx_destroy(evt_ctx);
		return 0;
	} else if (!strcmp(confs.run_type, "EV_INCREMENT")) {
		/* Current Running type is Synchronizing the increment data. */

		/* Send NET_EV_INCREMENT_REQ command to Server, for the first time. */
		strcpy(ev_des.id, confs.id);
		ev_des.ev_type = NET_EV_INCREMENT_REQ;
		ev_des.ev_state = NET_EV_NONE;
		ev_des.ev_size = 0;
		ev_des.incr.task_seq = 0;//FIXME	// First time, we don't know the task_seq.
		ev_des.incr.rows = 1;

		evt = copy_evt(&ev_des);
		print_evt(evt);
		assert(0 == send_evt(evt_ctx, evt));

		enum evt_type   last_type = NET_EV_INCREMENT_REQ;
		enum evt_state state = NET_EV_NONE;
		while (1) {
			/* Recv Server's response. */
			do {
				usleep(2000);
				evt = recv_evt(evt_ctx);
			} while(!evt);
			print_evt(evt);

			/* Doing businesses. return NET_EV_SUCC|NET_EV_FAIL|NET_EV_FATAL. */
			if ( !((last_type == NET_EV_DUMP_REQ) && (evt->ev_type == NET_EV_DUMP_REQ)) ) {
				state = do_business(evt->ev_type, errinfo, sizeof(errinfo), (evt->ev_size > 0) ? evt->ev_data : NULL, evt->ev_size);
			}
			last_type = evt->ev_type;
			/* Send Client Business execute state. */
			switch (last_type) {
				case BUSI_DUMP:
					evt->ev_type = NET_EV_DUMP_REP;
					evt->ev_state = state;
					evt->incr.rows = 0;
					evt->incr.task_seq = 0;

					print_evt(evt);
					assert(0 == send_evt(evt_ctx, evt));
					break;

				case BUSI_INCR:
					evt->ev_type = NET_EV_INCREMENT_REQ;
					evt->ev_state = state;
					evt->incr.rows = 1;
					evt->incr.task_seq = evt->incr.task_seq + 1;
					evt->ev_size = 0;

					print_evt(evt);
					assert(0 == send_evt(evt_ctx, evt));
					break;

				default:
					x_printf(E, "Invalid event type.\n");
					free_evt(evt);
					assert(0);
			}

			if (state != NET_EV_SUCC) {
				//严重错误时，程序退出
				sleep(2);
				x_printf(E, "send_result() fail. Error - %s.\n", evt_error(state));
				goto FAIL;
			}
		}
	} else {
		x_printf(E, "Invalid Running Type value <%s>\n", confs.run_type);
		return -1;
	}

	return 0;

FAIL:
	evt_ctx_destroy(evt_ctx);
	return -1;
}
