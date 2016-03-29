#include "zmq.h"
#include "xmq.h"
#include "ldb_cb.h"
#include "slog/slog.h"

void *data_write(void *data);

void *data_read(void *data);

int main()
{
	xmq_ctx_t *ctx = xmq_context_init("./ldb", 500000,
			ldb_pvt_create, driver_ldb_put, driver_ldb_get, ldb_pvt_destroy);

	if (!ctx) {
		x_printf(W, "Program initialize the zmq_context fail. exit!");
		return -1;
	}

	// xmq_register_producer(ctx, "P1");
	// xmq_register_consumer(ctx, "C1");

	// Write and read the data test.
	pthread_t thrd, thrd1, thrd2, thrd3;
	pthread_create(&thrd, NULL, data_write, ctx);
	sleep(1);
	// pthread_create(&thrd1, NULL, data_write, ctx);
	// sleep(1);
	pthread_create(&thrd2, NULL, data_read, ctx);
	sleep(1);
	pthread_create(&thrd3, NULL, data_read, ctx);
	sleep(1);

	pthread_join(thrd, NULL);
	// pthread_join(thrd1, NULL);
	pthread_join(thrd2, NULL);
	pthread_join(thrd3, NULL);

	// xmq_unregister_producer(ctx, "P1");
	// xmq_unregister_consumer(ctx, "C1");

	return 0;
}

void *data_read(void *data)
{
	static int index = 0;

	index++;
	xmq_ctx_t *ctx = (xmq_ctx_t *)data;

	char s_consumer[4];
	sprintf(s_consumer, "C%d", index);
	xmq_register_consumer(ctx, s_consumer);
	xmq_consumer_t *consumer = xmq_get_consumer(ctx, s_consumer);

	FILE *fp = fopen("./kvdata.log", "a+");

	uint64_t rindex = 1;

	while (1) {
		xmq_msg_t *msg = xmq_fetch_nth(consumer, rindex, -1);

		int res = fwrite(msg->data, msg->len, 1, fp);

		if (res != 1) {
			x_printf(E, "fwrite:(%s) fail. Error-%s.\n", msg->data, strerror(errno));
			xmq_msg_destroy(msg);
			fclose(fp);

			return NULL;
		}

		fflush(fp);

		xmq_msg_destroy(msg);

		rindex++;
	}

	fclose(fp);
}

void *data_write(void *data)
{
	static int index = 0;

	index++;
	xmq_ctx_t *ctx = (xmq_ctx_t *)data;

	char s_producer[4];
	sprintf(s_producer, "P%d", index);
	xmq_register_producer(ctx, s_producer);
	xmq_producer_t *producer = xmq_get_producer(ctx, s_producer);

	void    *zmqctx = zmq_ctx_new();
	void    *zmqsocket = zmq_socket(zmqctx, ZMQ_REP);
	int     rc = zmq_bind(zmqsocket, "tcp://*:8585");

	while (1) {
		zmq_msg_t zmqmsg;
		zmq_msg_init(&zmqmsg);

		rc = zmq_msg_recv(&zmqmsg, zmqsocket, 0);
		// x_printf(D, "zmq_msg_recv: %s\n", (char *)zmq_msg_data(&zmqmsg));

		if (-1 == rc) {
			break;
		}

		xmq_msg_t *xmqmsg = xmq_msg_new_data(zmq_msg_data(&zmqmsg), zmq_msg_size(&zmqmsg));
		zmq_msg_close(&zmqmsg);

		if (!xmqmsg) {
			break;
		}

		rc = xmq_push_tail(producer, xmqmsg);
		xmq_msg_destroy(xmqmsg);

		if (0 != rc) {
			break;
		}

		char response[] = "OK";
		rc = zmq_msg_init_data(&zmqmsg, response, sizeof(response), NULL, NULL);

		if (0 != rc) {
			break;
		}

		rc = zmq_msg_send(&zmqmsg, zmqsocket, 0);
		zmq_msg_close(&zmqmsg);

		if (rc != 3) {
			break;
		}
	}

	x_printf(I, "front thread stop!!!");

	zmq_close(zmqsocket);
	zmq_ctx_destroy(zmqctx);
	xmq_unregister_producer(ctx, s_producer);

	return NULL;
}

