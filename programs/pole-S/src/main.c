#include "netmod.h"
#include "conf.h"
#include "register.h"
#include "busi_db.h"
#include "busi_dump.h"
#include "busi_w2file.h"

int main(int argc, char *argv[])
{
	evt_t                   *evt;
	struct pole_conf        confs;

	/* Parsing configure file. */
	config_init(&confs, "./pole-S_conf.json");

	/* Initialize the Pole-S Log file. */
	SLogOpen(confs.log_file, SLogIntegerToLevel(confs.log_level));

	/* Initialize the Network. */
	evt_ctx_t *evt_ctx = evt_ctx_init(SOCK_CLIENT, confs.conn_uri, confs.self_uid);
	assert(evt_ctx);

	// Startup the thread of Network Center to Recv & Send data.
	pthread_t       thrd;
	pthread_attr_t  attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int ok = pthread_create(&thrd, &attr, work_evt, evt_ctx);
	assert(ok == 0);

	/* Register the business functions.
	 * busi_dump you should never modify it.
	 * busi_incr you can modify it's elements with your own.
	 */
	business_t      busi_dump = {
		dump_init,
		dump_db,
		NULL
	};
	business_t      busi_incr = {
		w2file_init,
		w2file_done,
		w2file_destroy
	};

	register_business(BUSI_DUMP, &busi_dump);
	register_business(BUSI_INCR, &busi_incr);

	/* Startup all the businesses. */
	int res = startup_businesses(&confs);
	assert(res == 0);

	/* Execute by current running type. */
	if (!strcmp(confs.run_type, "DUMP")) {
		/* Current Running type is Synchronizing the database base data. */

		/* Send NET_EV_DUMP_REQ command to Server. */
		int len = strlen(confs.dump_uid) + 1;

		if (len == 1) {
			x_printf(W, "The pole-S_conf.json file DEST_DUMP_UID should be set,"
				" when event type is DUMP.\n");
			goto FAIL;
		}

		evt = evt_new_by_size(len);
		assert(evt);

		strcpy(evt->id, confs.self_uid);
		evt->ev_type = NET_EV_DUMP_REQ;
		evt->ev_state = NET_EV_NONE;
		evt->ev_size = len;
		strcpy(evt->ev_data, confs.dump_uid);

		print_evt(evt);
		assert(0 == send_evt(evt_ctx, evt));

		/* Recv NET_EV_DUMP_REP command from Server. */
		/* We'll wait for a long time, Because Dump DB is very consume time. */
		do {
			sleep(1);
			evt = recv_evt(evt_ctx);
		} while (!evt);

		print_evt(evt);

		if (evt->ev_state == NET_EV_FAIL) {
			x_printf(E, "Destination Host, dump the MySQL database data fail.\n");
			goto FAIL;
		}

		x_printf(I, "Destination Host, has already dumped the MySQL database succeed.\n");
		free_evt(evt);

		/* Process exit. */
		evt_ctx_destroy(evt_ctx);
	} else if (!strcmp(confs.run_type, "INCR")) {
		/* Current Running type is Synchronizing the increment data. */

		/* Send NET_EV_INCREMENT_REQ command to Server, for the first time. */
		evt = evt_new_by_size(0);
		assert(evt);
		strcpy(evt->id, confs.self_uid);
		evt->ev_type = NET_EV_INCREMENT_REQ;
		evt->ev_state = NET_EV_NONE;
		evt->ev_size = 0;
		evt->incr.task_seq = confs.incr_seq;	// First time, if we don't know the task_seq, set to 0.
		evt->incr.rows = 1;

		print_evt(evt);
		assert(0 == send_evt(evt_ctx, evt));

		enum evt_type   last_type = NET_EV_INCREMENT_REQ;
		enum evt_state  state = NET_EV_NONE;
		uint64_t        last_seq = 0;

		while (true) {
			/* Recv Server's response. */
			do {
				usleep(2000);
				evt = recv_evt(evt_ctx);
			} while (!evt);
			print_evt(evt);

			// 避免连续重复incr
			if (evt->ev_type == NET_EV_INCREMENT_REP) {
				if (evt->incr.task_seq == last_seq) {
					free_evt(evt);
					continue;
				} else {
					last_seq = evt->incr.task_seq;
				}
			}

			// 避免连续重复dump
			if (!((last_type == NET_EV_DUMP_REQ) && (evt->ev_type == NET_EV_DUMP_REQ))) {
				/* Doing businesses. return NET_EV_SUCC|NET_EV_FAIL|NET_EV_FATAL. */
				state = do_business(evt->ev_type, (evt->ev_size > 0) ? evt->ev_data : NULL, evt->ev_size);
			}

			last_type = evt->ev_type;
			/* Send Client Business execute state. */
			switch (last_type)
			{
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
				// 严重错误时，程序退出
				sleep(2);
				x_printf(E, "*******************************.\n");
				goto FAIL;
			}
		}
	} else {
		x_printf(E, "Invalid Running Type value <%s>\n", confs.run_type);
		goto FAIL;
	}

	return 0;

FAIL:
	evt_ctx_destroy(evt_ctx);
	return -1;
}

