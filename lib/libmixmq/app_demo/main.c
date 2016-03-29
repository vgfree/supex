#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mixmq.h"
#include "ldb_cb.h"

void csv_test(const char *s)
{
	printf("\ncsv_test() start>>>>>>>>>>>>>>>>>>>>>>\n");

	csv_parser_t csv_parser;
	xmq_csv_parser_init(&csv_parser);
	xmq_csv_parse_string(&csv_parser, s);

	printf("[%s]-----------[%d]\n", csv_string(&csv_parser), csv_field_num(&csv_parser));

	int             i = 0;
	csv_field_t     *field;
	for_each_field(field, &csv_parser)
	{
		char *tmp = xmq_csv_field_to_string(field);

		printf("%d:  [%s]\n", i++, tmp);
		free(tmp);
	}

	xmq_csv_parser_destroy(&csv_parser);

	printf("csv_test() end<<<<<<<<<<<<<<<<<<<<<<<\n\n");
}

void msg_print(xmq_msg_t *msg)
{
	printf("\nmsg_print() start>>>>>>>>>>>>>>>>>>>>>>\n");

	printf("     message head size:     %-lu\n", MSG_HEAD_SIZE);
	printf("     message body size:     %-ld\n", xmq_msg_size(msg));
	printf("    message total size:     %-lu\n", xmq_msg_total_size(msg));

	printf("          message body:     %-s\n", (char *)xmq_msg_data(msg));
	printf("      message body len:     %-ld\n", strlen((const char *)xmq_msg_data(msg)));

	printf("msg_print() end<<<<<<<<<<<<<<<<<<<<<<<\n\n");
}

void msg_test(const void *data, size_t size)
{
	xmq_msg_t *msg = xmq_msg_new_data(data, size);

	msg_print(msg);

	xmq_msg_t *copy = xmq_msg_dup(msg);
	msg_print(copy);

	xmq_msg_destroy(msg);
	xmq_msg_destroy(copy);
}

xmq_context_t *g_ctx;

#define foreach_node_start(node, head, type, entry)		       \
	for (node = (head)->next; node != (head); node = node->next) { \
		entry = container_of(node, type, list);

#define foreach_node_end \
	}

void queue_test(void)
{
	printf("\nqueue_test() start>>>>>>>>>>>>>>>>>>>>>>\n");

	xmq_queue_t     *tmp;
	list_t          *iter;

	printf("dead queue:\n");
	foreach_node_start(iter, &g_ctx->dead_queues, xmq_queue_t, tmp)
	printf("queue: %s\n", tmp->name);
	foreach_node_end

	xmq_queue_t     *q1 = xmq_queue_open(g_ctx, "first", 0);
	xmq_queue_t     *q2 = xmq_queue_open(g_ctx, "second", 0);
	xmq_queue_t     *q3 = xmq_queue_open(g_ctx, "three", 0);
	xmq_queue_t     *q4 = xmq_queue_open(g_ctx, "four", 0);

	printf("active queue:\n");
	foreach_node_start(iter, &g_ctx->active_queues, xmq_queue_t, tmp)
	printf("queue: %s\n", tmp->name);
	foreach_node_end

	xmq_queue_close(q1);

	xmq_queue_close(q2);
	xmq_queue_close(q3);
	xmq_queue_close(q4);

	printf("dead queue:\n");
	foreach_node_start(iter, &g_ctx->dead_queues, xmq_queue_t, tmp)
	printf("queue: %s\n", tmp->name);
	foreach_node_end

	printf("queue_test() end<<<<<<<<<<<<<<<<<<<<<<<\n\n");
}

void producer_test(void)
{
	printf("\nproducer_test() start>>>>>>>>>>>>>>>>>>>>>>\n");
	xmq_queue_t *q1 = xmq_queue_open(g_ctx, "first", 0);

	xmq_producer_t  *tmp;
	list_t          *iter;

	printf("dead_producers:\n");
	foreach_node_start(iter, &q1->dead_producers, xmq_producer_t, tmp)
	printf("producer: %s\n", tmp->identity);
	foreach_node_end

	xmq_producer_t  *p1 = xmq_register_as_producer(q1, "ID_a");
	xmq_producer_t  *p2 = xmq_register_as_producer(q1, "ID_b");
	xmq_producer_t  *p3 = xmq_register_as_producer(q1, "ID_c");
	xmq_producer_t  *p4 = xmq_register_as_producer(q1, "ID_d");

	printf("active_producers:\n");
	foreach_node_start(iter, &q1->active_producers, xmq_producer_t, tmp)
	printf("producer: %s\n", tmp->identity);
	foreach_node_end

	if (p1) {
		xmq_unregister_producer(q1, "ID_a");
	}

	if (p2) {
		xmq_unregister_producer(q1, "ID_b");
	}

	if (p3) {
		xmq_unregister_producer(q1, "ID_c");
	}

	if (p4) {
		xmq_unregister_producer(q1, "ID_d");
	}

	printf("dead_producers:\n");
	foreach_node_start(iter, &q1->dead_producers, xmq_producer_t, tmp)
	printf("producer: %s\n", tmp->identity);
	foreach_node_end

	xmq_queue_close(q1);

	printf("producer_test() end<<<<<<<<<<<<<<<<<<<<<<<\n\n");
}

void consumer_test(void)
{
	printf("\nconsumer_test() start>>>>>>>>>>>>>>>>>>>>>>\n");
	xmq_queue_t *q1 = xmq_queue_open(g_ctx, "first", 0);

	xmq_consumer_t  *tmp;
	list_t          *iter;

	printf("dead_consumers:\n");
	foreach_node_start(iter, &q1->dead_consumers, xmq_consumer_t, tmp)
	printf("producer: %s\n", tmp->identity);
	foreach_node_end

	xmq_consumer_t  *c1 = xmq_register_as_consumer(q1, "ID_a");
	xmq_consumer_t  *c2 = xmq_register_as_consumer(q1, "ID_b");
	xmq_consumer_t  *c3 = xmq_register_as_consumer(q1, "ID_c");
	xmq_consumer_t  *c4 = xmq_register_as_consumer(q1, "ID_d");

	printf("active_consumers:\n");
	foreach_node_start(iter, &q1->active_consumers, xmq_consumer_t, tmp)
	printf("producer: %s\n", tmp->identity);
	foreach_node_end

	if (c1) {
		xmq_unregister_consumer(q1, "ID_a");
	}

	if (c2) {
		xmq_unregister_consumer(q1, "ID_b");
	}

	if (c3) {
		xmq_unregister_consumer(q1, "ID_c");
	}

	if (c4) {
		xmq_unregister_consumer(q1, "ID_d");
	}

	printf("dead_consumers:\n");
	foreach_node_start(iter, &q1->dead_consumers, xmq_consumer_t, tmp)
	printf("producer: %s\n", tmp->identity);
	foreach_node_end

	xmq_queue_close(q1);

	printf("consumer_test() end<<<<<<<<<<<<<<<<<<<<<<<\n\n");
}

void msg_io_test(void)
{
	printf("\nmsg_io_test() start>>>>>>>>>>>>>>>>>>>>>>\n");
	xmq_queue_t *q1 = xmq_queue_open(g_ctx, "first", 1);

#if 1
	xmq_producer_t  *p1 = xmq_register_as_producer(q1, "ID_a");
	int             i;

	for (i = 1; i <= 10; i++) {
		char info[64];
		snprintf(info, sizeof(info), "hello world-%d", i);

		xmq_msg_t *msg = xmq_msg_new_data(info, strlen(info) + 1);
		queue_push_tail(p1, msg);
	}
#endif

	xmq_consumer_t  *c2 = xmq_register_as_consumer(q1, "ID_b");
	xmq_msg_t       *msg = queue_fetch_nth(c2, 1, 0);
	do {
		printf("data:%s----%lu\n", (char *)xmq_msg_data(msg), xmq_msg_index(msg));
	} while ((xmq_msg_t *)-1 != (msg = queue_fetch_next(c2, 0)));
	xmq_unregister_consumer(q1, "ID_b");

	xmq_queue_close(q1);
	printf("msg_io_test() end<<<<<<<<<<<<<<<<<<<<<<<\n\n");
}

int main(void)
{
	g_ctx = xmq_context_new();
	ldb_pvt_t *ldb_pvt = ldb_pvt_create("./data");

	xmq_context_init(g_ctx, ldb_pvt, ldb_pvt_destroy, driver_ldb_put, driver_ldb_get);

	msg_io_test();

	xmq_context_term(g_ctx);

	return 0;
}

