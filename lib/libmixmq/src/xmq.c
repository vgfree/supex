#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "xmq.h"
#include "xmq_csv.h"
#include "slog/slog.h"

static char *__string_append_3rd(const char *prfix, const char *sep,
	const char *first, const char *second, const char *three);

static int __load_string_bin(xmq_context_t *ctx, const char *string_key, void **bin_value, unsigned int *bin_len);

static char *__load_string_string(xmq_context_t *ctx, const char *string_key);

static int __write_string_bin(xmq_context_t *ctx, const char *string_key, const void *bin_value, unsigned int bin_len);

static int __write_string_string(xmq_context_t *ctx, const char *string_key, const char *string_value);

static int __list_to_csv(list_t *head, csv_parser_t *csv, unsigned int type_code);

static int __csv_to_list(csv_parser_t *csv, list_t *head, unsigned int type_code, void *parent);

static int __load_string_list(xmq_context_t *ctx, const char *string_key,
	list_t *head, unsigned int type_code, void *parent);

static char *__write_back_append(xmq_context_t *ctx, const char *string_key,
	const char *more_part, const char *sep);

static int __uint64_read(xmq_context_t *ctx, char *key, uint64_t *u64num);

static int __uint64_write(xmq_context_t *ctx, char *key, uint64_t u64num);

static int __uint64_test_init(xmq_context_t *ctx, char *key, uint64_t *cur, uint64_t init);

static int __queues_load(xmq_context_t *ctx);

static int __producers_load(xmq_queue_t *queue);

static int __consumers_load(xmq_queue_t *queue);

static xmq_queue_t *__queue_active(xmq_queue_t *queue);

static xmq_queue_t *__queue_new_active(xmq_context_t *ctx, const char *name);

static xmq_consumer_t *__consumer_active(xmq_consumer_t *consumer);

static xmq_consumer_t *__consumer_new_active(xmq_queue_t *queue, const char *identity);

static xmq_msg_t *__msg_fetch(xmq_consumer_t *consumer, uint64_t index);

#undef min
#define min(x, y)       (((x) <= (y)) ? (x) : (y))

#undef max
#define max(x, y)       (((x) >= (y)) ? (x) : (y))

#undef freeif
#define freeif(p)	      \
	do { if (p) {	      \
		     free(p); \
	     }		      \
	     p = NULL; } while (0)

#undef return_if_false
#define return_if_false(v, r)						     \
	do { if (!(v)) {						     \
		     x_printf(W, "expression [ " #v " ]" " value is false"); \
		     return (r);					     \
	     }								     \
	} while (0)

/*__string_append_3rd - 拼接最多三个字符串，组成一个新的字符串
 *                      拼接格式  prfix + first + sep + second + sep + three + sep
 * @prfix:              前缀，可选
 * @sep:                分隔符，可选
 * @first:              第一个字符串
 * @second:             第二个字符串，如果first != NULL
 * @three:              第三个字符串，如果second != NULL
 * return:              成功返回新的字符串(需要free释放)，失败返回NULL
 * */
static char *__string_append_3rd(const char *prfix, const char *sep,
	const char *first, const char *second, const char *three)
{
	size_t total = 1;

	if (prfix) {
		total += strlen(prfix);
	}

	if (sep) {
		total += (2 * strlen(sep));
	}

	if (first) {
		total += strlen(first);

		if (second) {
			total += strlen(second);

			if (three) {
				total += strlen(three);
			}
		}
	}

	char *merge = (char *)calloc(1, total);
	return_if_false(merge, NULL);

	if (merge) {
		if (prfix) {
			strcat(merge, prfix);
		}

		if (first) {
			strcat(merge, first);

			if (sep) {
				strcat(merge, sep);
			}

			if (second) {
				strcat(merge, second);

				if (sep) {
					strcat(merge, sep);
				}

				if (three) {
					strcat(merge, three);

					if (sep) {
						strcat(merge, sep);
					}
				}
			}
		}
	}

	return merge;
}

#define get_key_of_queues()             __string_append_3rd("__XMQ__", "__", "QUEUES", NULL, NULL)
#define get_key_of_producers(q)         __string_append_3rd("__XMQ__", "__", (q), "PRODUCERS", NULL)
#define get_key_of_consumers(q)         __string_append_3rd("__XMQ__", "__", (q), "CONSUMERS", NULL)
#define get_key_of_capacity(q)          __string_append_3rd("__XMQ__", "__", (q), "CAPACITY", NULL)
#define get_key_of_fetchpos(q, u)       __string_append_3rd("__XMQ__", "__", (q), (u), "FETCHPOS")
#define get_key_of_markpos(q, u)        __string_append_3rd("__XMQ__", "__", (q), (u), "MARKPOS")

/*__load_string_bin - 以字符串为KEY，加载二进制VALUE
 * @ctx:              上下文结构指针
 * @string_key        字符串KEY
 * @bin_value         返回值参数，返回二进制VALUE 缓存指针，无对应VALUE 值返回NULL，错误无意义
 * @bin_len           返回值参数，返回缓存二进制VALUE 长度，无对应VALUE 值返回0，错误无意义
 * return:            成功返回0，失败返回-1
 * */
static int __load_string_bin(xmq_context_t *ctx, const char *string_key, void **bin_value, unsigned int *bin_len)
{
	return_if_false((ctx && ctx->get_KVs_cb), -1);
	return_if_false((string_key && bin_value && bin_len), -1);

	binary_entry_t bin_k, bin_v;
	set_bin_entry(&bin_k, string_key, strlen(string_key));

	int res = ctx->get_KVs_cb(ctx->pvt, &bin_k, &bin_v, 1);

	if (res != -1) {
		*bin_value = bin_v.data;
		*bin_len = bin_v.len;
	}

	return res;
}

/*__load_string_string - 以字符串为KEY，将VALUE 作为字符串返回
 * @ctx:                 上下文结构指针
 * @string_key:          字符串KEY
 * return:               成功返回VALUE 指针，无对应VALUE 值返回NULL，失败返回(char *)-1
 * */
static char *__load_string_string(xmq_context_t *ctx, const char *string_key)
{
	void            *bin_value;
	unsigned int    bin_len;

	if (-1 == __load_string_bin(ctx, string_key, &bin_value, &bin_len)) {
		return (char *)-1;
	}

	return (char *)bin_value;
}

/*__write_string_bin - 写入字符串KEY-二进制VALUE
 * @ctx:               上下文结构指针
 * @string_key:        字符串KEY
 * @bin_value:         二进制VALUE首地址
 * @bin_len:           二进制VALUE长度
 * return:             成功返回0，失败返回-1
 * */
static int __write_string_bin(xmq_context_t *ctx, const char *string_key, const void *bin_value, unsigned int bin_len)
{
	return_if_false((ctx && ctx->put_KVs_cb), -1);
	return_if_false((string_key && bin_value && bin_len), -1);

	binary_entry_t bin_k, bin_v;
	set_bin_entry(&bin_k, string_key, strlen(string_key));
	set_bin_entry(&bin_v, bin_value, bin_len);

	return ctx->put_KVs_cb(ctx->pvt, &bin_k, &bin_v, 1);
}

/*__write_string_string - 写入字符串KEY-字符串VALUE
 * @ctx:                  上下文结构指针
 * @string_key            字符串KEY
 * @string_value          字符串VALUE
 * return:                成功返回0，失败返回-1
 * */
static int __write_string_string(xmq_context_t *ctx, const char *string_key, const char *string_value)
{
	return __write_string_bin(ctx, string_key, string_value, strlen(string_value) + 1);
}

#define TYPE_XMQ_QUEUE          0x1
#define TYPE_XMQ_PRODUCER       0x2
#define TYPE_XMQ_CONSUMER       0x3

/*__list_to_csv - 将链表节点名拼接成一个CSV字符串并解析
 *                如果节点类型为xmq_queue_t，那么节点名为name
 *                如果节点类型为xmq_producer_t，那么节点名为identity
 *                如果节点类型为xmq_consumer_t，那么节点名为identity
 * @head:         链表头指针
 * @csv:          csv解析器指针
 * @type_code:    指定节点类型
 * return:        返回节点个数，失败返回-1
 * */
static int __list_to_csv(list_t *head, csv_parser_t *csv, unsigned int type_code)
{
	return_if_false((head && !list_empty(head) && csv), -1);

	char *cur, *more, *tmp;
	cur = more = tmp = NULL;

	list_t *node;
	list_for_each(node, head)
	{
		switch (type_code)
		{
			case TYPE_XMQ_QUEUE:
			{
				xmq_queue_t *queue = container_of(node, xmq_queue_t, list);
				more = queue->name;
			}
			break;

			case TYPE_XMQ_PRODUCER:
			{
				xmq_producer_t *producer = container_of(node, xmq_producer_t, list);
				more = producer->identity;
			}
			break;

			case TYPE_XMQ_CONSUMER:
			{
				xmq_consumer_t *consumer = container_of(node, xmq_consumer_t, list);
				more = consumer->identity;
			}
			break;

			default:
			{
				x_printf(E, "type_code [%d] is invalid", type_code);
				freeif(cur);
				return -1;
			}
		}

		tmp = __string_append_3rd(cur, ",", more, NULL, NULL);

		freeif(cur);
		return_if_false(tmp, -1);
		cur = tmp;
	}

	int res = xmq_csv_parse_string(csv, cur);
	free(cur);

	return res;
}

/*__csv_to_list - 将CSV字符串解析，各字段作为节点名创建一个链表
 *                如果节点类型为xmq_queue_t，那么节点名为name
 *                如果节点类型为xmq_producer_t，那么节点名为identity
 *                如果节点类型为xmq_consumer_t，那么节点名为identity
 * @csv:          csv解析器指针，必须通过解析生成结果集
 * @head:         链表头指针
 * @type_code:    指定节点类型
 * @parent:       父节点
 * return:        返回节点个数，失败返回-1
 * */
static int __csv_to_list(csv_parser_t *csv, list_t *head, unsigned int type_code, void *parent)
{
	return_if_false((csv && head && list_empty(head)), -1);

	csv_field_t     *field;
	unsigned int    index = 0;

	for_each_field(field, csv)
	{
		switch (type_code)
		{
			case TYPE_XMQ_QUEUE:
			{
				xmq_queue_t *queue = (xmq_queue_t *)calloc(1, sizeof(xmq_queue_t));

				if (!queue) {
					list_del_all_entrys(head, xmq_queue_t, list);
					return -1;
				}

				list_add_tail(&queue->list, head);
				strncpy(queue->name, field->ptr, sizeof(queue->name));
				queue->name[min(sizeof(queue->name), field->len)] = '\0';
				queue->context = (xmq_context_t *)parent;

				list_init(&queue->dead_producers);
				list_init(&queue->dead_consumers);
				list_init(&queue->active_producers);
				list_init(&queue->active_consumers);

				queue->id = index++;
			}
			break;

			case TYPE_XMQ_PRODUCER:
			{
				xmq_producer_t *producer = (xmq_producer_t *)calloc(1, sizeof(xmq_producer_t));

				if (!producer) {
					list_del_all_entrys(head, xmq_producer_t, list);
					return -1;
				}

				list_add_tail(&producer->list, head);
				strncpy(producer->identity, field->ptr, sizeof(producer->identity));
				producer->identity[min(sizeof(producer->identity), field->len)] = '\0';
				producer->owner_queue = (xmq_queue_t *)parent;

				index++;
			}
			break;

			case TYPE_XMQ_CONSUMER:
			{
				xmq_consumer_t *consumer = (xmq_consumer_t *)calloc(1, sizeof(xmq_consumer_t));

				if (!consumer) {
					list_del_all_entrys(head, xmq_consumer_t, list);
					return -1;
				}

				list_add_tail(&consumer->list, head);
				strncpy(consumer->identity, field->ptr, sizeof(consumer->identity));
				consumer->identity[min(sizeof(consumer->identity), field->len)] = '\0';
				consumer->owner_queue = (xmq_queue_t *)parent;

				consumer->slot = index++;
			}
			break;

			default:
			{
				printf("__csv_to_list: type_code [%d] is invalid.\n", type_code);
				return -1;
			}
		}
	}	// for cycle.

	return index;
}

/*__load_string_list - 字符串KEY 加载一个CSV字符串VALUE，并转换成链表
 * @ctx:               上下文结构指针
 * @string_key:        字符串KEY
 * @type_code:         节点类型
 * @parent:            父节点指针
 * @node_number:       返回链表节点个数
 * return:
 * 成功返回节点个数，错误返回-1
 * */
static int __load_string_list(xmq_context_t *ctx, const char *string_key,
	list_t *head, unsigned int type_code, void *parent)
{
	return_if_false((ctx && ctx->get_KVs_cb), -1);
	return_if_false((head && list_empty(head) && string_key), -1);

	char *str_value = __load_string_string(ctx, string_key);
	return_if_false((str_value != (char *)-1), -1);
	return_if_false(str_value, 0);

	csv_parser_t csv;
	xmq_csv_parser_init(&csv);
	int n = xmq_csv_parse_string(&csv, str_value);
	free(str_value);
	return_if_false((n != -1), -1);

	if (-1 == __csv_to_list(&csv, head, type_code, parent)) {
		printf("__load_string_list: __csv_to_list failed.\n");
		xmq_csv_parser_destroy(&csv);
		return -1;
	}

	xmq_csv_parser_destroy(&csv);
	return n;
}

/*__write_back_append - 根据KEY 在现有VALUE基础上追加，并回写到此KEY
 * @ctx:                上下文结构指针
 * @string_key:         字符串KEY
 * @more_part:          需要追加的字符串
 * @sep:                分隔符
 * return:              成功返回原VALUE，原VALUE 为空值返回NULL，失败返回(char *)-1
 * */
static char *__write_back_append(xmq_context_t *ctx, const char *string_key,
	const char *more_part, const char *sep)
{
	char *value_old = __load_string_string(ctx, string_key);

	return_if_false((value_old != (char *)-1), (char *)-1);

	char *value_new = __string_append_3rd(value_old, sep, more_part, NULL, NULL);

	if (!value_new) {
		free(value_old);
		return (char *)-1;
	}

	int rc = __write_string_string(ctx, string_key, value_new);

	if (-1 == rc) {
		free(value_old);
		free(value_new);
		return (char *)-1;
	}

	free(value_new);
	return value_old;
}

/*__uint64_read - 读取KEY对应的uint64_t整数
 * @ctx:          上下文结构指针
 * @key:          字符串KEY
 * @u64num:       uint64_t类型指针，存储成功读取的整数
 * return:        失败返回-1，KEY无对应VALUE返回0，成功返回正数
 * */
static int __uint64_read(xmq_context_t *ctx, char *key, uint64_t *u64num)
{
	uint64_t        *u64;
	unsigned int    len;
	int             res = __load_string_bin(ctx, key, (void **)&u64, &len);

	if (-1 != res) {
		if (u64) {
			return_if_false((sizeof(uint64_t) == len), -1);
			*u64num = *u64;
			free(u64);

			res++;
		}
	}

	return res;
}

/*__uint64_write - 将uint64_t整数写入到KEY
 * @ctx:           上下文结构指针
 * @key:           字符串KEY
 * @u64num:        要写入的uint64_t整数
 * return:         失败返回-1，成功返回0
 * */
static int __uint64_write(xmq_context_t *ctx, char *key, uint64_t u64num)
{
	return __write_string_bin(ctx, key, (void *)&u64num, (unsigned int)sizeof(uint64_t));
}

/*__uint64_test_init - 测试KEY对应的uint64_t整数是否有值，无值则初始化
 * @ctx:               上下文结构指针
 * @key:               字符串KEY
 * @cur:               如果有值，存储当前值
 * @init:              如果无值，将要初始化的值
 * return:             如果有值并成功读取返回1，如果无值并成功写入返回0，错误返回-1
 * */
static int __uint64_test_init(xmq_context_t *ctx, char *key, uint64_t *cur, uint64_t init)
{
	int res = __uint64_read(ctx, key, cur);

	if (0 == res) {
		*cur = init;
		res = __uint64_write(ctx, key, init);
	}

	return res;
}

/*__queues_load - 加载队列
 * @ctx:          上下文结构指针
 * return:        成功返回0，失败返回-1*/
static int __queues_load(xmq_context_t *ctx)
{
	return_if_false(ctx, -1);
	return_if_false(ctx->get_KVs_cb, -1);

	char *queues_key = get_key_of_queues();
	return_if_false(queues_key, -1);

	ctx->queue_amount = __load_string_list(ctx, queues_key, &ctx->dead_queues, TYPE_XMQ_QUEUE, ctx);
	free(queues_key);

	if (-1 == ctx->queue_amount) {
		ctx->queue_amount = 0;
		return -1;
	}

	return 0;
}

/*__producers_load - 加载队列生产者
 * @queue:           队列指针，必须已打开
 * return:           成功返回0，失败返回-1*/
static int __producers_load(xmq_queue_t *queue)
{
	return_if_false(queue, -1);
	return_if_false(queue->context, -1);
	return_if_false(queue->context->get_KVs_cb, -1);

	char *producers_key = get_key_of_producers(queue->name);
	return_if_false(producers_key, -1);

	queue->producer_amount = __load_string_list(queue->context, producers_key,
			&queue->dead_producers, TYPE_XMQ_PRODUCER, queue);
	free(producers_key);

	if (-1 == queue->producer_amount) {
		queue->producer_amount = 0;
		return -1;
	}

	return 0;
}

/*__consumers_load - 加载队列消费者
 * @queue:           队列指针，必须已打开
 * return:           成功返回0，失败返回-1
 * */
static int __consumers_load(xmq_queue_t *queue)
{
	return_if_false(queue, -1);
	return_if_false(queue->context, -1);
	return_if_false(queue->context->get_KVs_cb, -1);

	char *consumers_key = get_key_of_consumers(queue->name);
	return_if_false(consumers_key, -1);

	queue->consumer_amount = __load_string_list(queue->context, consumers_key,
			&queue->dead_consumers, TYPE_XMQ_CONSUMER, queue);
	free(consumers_key);

	if (-1 == queue->consumer_amount) {
		queue->consumer_amount = 0;
		return -1;
	}

	return 0;
}

/*xmq_context_new - 动态内存分配一个上下文
 * return:          成功返回上下文结构类型指针，失败返回NULL
 * */
extern xmq_context_t *xmq_context_new(void)
{
	xmq_context_t *ctx = (xmq_context_t *)malloc(sizeof(xmq_context_t));

	if (ctx) {
		memset(ctx, 0, sizeof(xmq_context_t));
	}

	return ctx;
}

/*xmq_context_init - 使用存储媒介相关的私有数据、回调函数初始化上下文结构，并从存储媒介中载入队列相关信息
 * @ctx:             需要初始化的上下文
 * @pvt:             媒介相关用户私用数据
 * @pvt_free:        释放用户私有数据回调
 * return:           成功返回0，失败返回-1
 * */
extern int xmq_context_init(xmq_context_t *ctx, void *pvt, pvt_free_fn *pvt_free,
	put_KVs_fn put_kvs, get_KVs_fn get_kvs)
{
	return_if_false(ctx, -1);

	ctx->pvt = pvt;

	if (NULL != pvt) {
		return_if_false(pvt_free, -1);
	}

	ctx->pvt_free_cb = pvt_free;
	ctx->put_KVs_cb = put_kvs;
	ctx->get_KVs_cb = get_kvs;

	list_init(&ctx->dead_queues);
	list_init(&ctx->active_queues);

	pthread_mutex_init(&ctx->node_tree_lock, NULL);

	return __queues_load(ctx);
}

/*xmq_context_term - 释放上下文，队列必须全部closed
 *                   如果当前还有活动的队列将会导致失败
 * @ctx:             需要释放的上下文结构指针
 * return:           成功返回0，失败返回-1
 * */
extern int xmq_context_term(xmq_context_t *ctx)
{
	return_if_false(ctx, -1);
	pthread_mutex_lock(&ctx->node_tree_lock);

	if (!list_empty(&ctx->active_queues)) {
		pthread_mutex_unlock(&ctx->node_tree_lock);
		return -1;
	}

	if (ctx->pvt) {
		int n = ctx->pvt_free_cb(ctx->pvt);

		if (-1 == n) {
			pthread_mutex_unlock(&ctx->node_tree_lock);
			return -1;
		}
	}

	list_del_all_entrys(&ctx->dead_queues, xmq_queue_t, list);

	pthread_mutex_unlock(&ctx->node_tree_lock);
	pthread_mutex_destroy(&ctx->node_tree_lock);

	return 0;
}

/*xmq_queue_is_active - 队列是否活动
 * @ctx:                上下文结构指针
 * @qname:              查看是否活动的队列名
 * return:              活动返回队列指针，否则返回NULL
 * */
extern xmq_queue_t *xmq_queue_is_active(xmq_context_t *ctx, const char *qname)
{
	xmq_queue_t *queue = NULL;

	return_if_false((ctx && qname), queue);

	pthread_mutex_lock(&ctx->node_tree_lock);

	list_t *iter;
	list_for_each(iter, &ctx->active_queues)
	{
		xmq_queue_t *q = container_of(iter, xmq_queue_t, list);

		if (!strncmp(q->name, qname, strlen(qname) + 1)) {
			queue = q;
			break;
		}
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);

	return queue;
}

/*xmq_queue_exist - 队列是否存在
 * @ctx:            上下文结构指针
 * @qname:          查看是否存在的队列名
 * return:          存在返回队列地址，否则返回NULL
 * */
extern xmq_queue_t *xmq_queue_exist(xmq_context_t *ctx, const char *qname)
{
	xmq_queue_t *queue = NULL;

	return_if_false((ctx && qname), queue);

	if ((queue = xmq_queue_is_active(ctx, qname))) {
		return queue;
	}

	pthread_mutex_lock(&ctx->node_tree_lock);

	list_t *iter;
	list_for_each(iter, &ctx->dead_queues)
	{
		xmq_queue_t *q = container_of(iter, xmq_queue_t, list);

		if (!strncmp(q->name, qname, strlen(qname) + 1)) {
			queue = q;
			break;
		}
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);

	return queue;
}

/*__queue_active - 将一个dead QUEUE 激活
 * @queue:         要激活的队列
 * return:         成功返回激活队列，失败返回NULL
 * */
static xmq_queue_t *__queue_active(xmq_queue_t *queue)
{
	return_if_false((queue && queue->context), NULL);
	xmq_context_t *ctx = queue->context;

	char *capacity_key = get_key_of_capacity(queue->name);
	return_if_false(capacity_key, NULL);

	int rc = __uint64_test_init(queue->context, capacity_key, &queue->msg_capacity, (uint64_t)0);
	freeif(capacity_key);
	return_if_false((rc != -1), NULL);

	if (list_empty(&queue->dead_producers)) {
		return_if_false((0 == __producers_load(queue)), NULL);
	}

	if (list_empty(&queue->dead_consumers)) {
		return_if_false((0 == __consumers_load(queue)), NULL);
	}

	list_del(&queue->list);
	list_add_tail(&queue->list, &ctx->active_queues);

	return queue;
}

/*__queue_new_active - 创建一个队列并激活
 * @ctx:               上下文结构指针
 * @name:              新创建的队列名
 * return:             成功返回激活队列，失败返回NULL
 * */
static xmq_queue_t *__queue_new_active(xmq_context_t *ctx, const char *name)
{
	xmq_queue_t *queue = (xmq_queue_t *)calloc(1, sizeof(xmq_queue_t));

	if (queue) {
		strncpy(queue->name, name, strlen(name) + 1);
		queue->context = ctx;
		list_add_tail(&queue->list, &ctx->dead_queues);

		list_init(&queue->dead_producers);
		list_init(&queue->dead_consumers);
		list_init(&queue->active_producers);
		list_init(&queue->active_consumers);

		if (NULL == __queue_active(queue)) {
			free(queue);
			return NULL;
		}

		char *queues_key = get_key_of_queues();

		if (!queues_key) {
			list_del(&queue->list);
			free(queue);
			return NULL;
		}

		char *old_queues = __write_back_append(ctx, queues_key, queue->name, ",");
		free(queues_key);

		if (old_queues == (char *)-1) {
			list_del(&queue->list);
			free(queue);
			return NULL;
		}

		freeif(old_queues);

		queue->id = ctx->queue_amount;
		ctx->queue_amount++;
	}

	return queue;
}

/*xmq_queue_open - 打开一个队列，如果不存在则创建
 * @ctx:           上下文结构指针
 * @queue:         队列名，唯一标识一个队列
 * @flag:          标识符，当前主要标记阻塞非阻塞
 * return:         队列结构类型，失败返回NULL
 * */
extern xmq_queue_t *xmq_queue_open(xmq_context_t *ctx, const char *name, unsigned int flag)
{
	return_if_false((ctx && name), NULL);
	return_if_false((strlen(name) < 256), NULL);
	xmq_queue_t *queue = NULL;

	if ((queue = xmq_queue_is_active(ctx, name))) {
		x_printf(W, "queue [%s] already has been opened", name);
	} else if ((queue = xmq_queue_exist(ctx, name))) {
		pthread_mutex_lock(&ctx->node_tree_lock);
		queue = __queue_active(queue);
	} else {
		pthread_mutex_lock(&ctx->node_tree_lock);
		queue = __queue_new_active(ctx, name);
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);

	if (queue) {
		queue->flag = flag;

		pthread_mutex_init(&queue->wait_lock, NULL);
		pthread_cond_init(&queue->wait_cond, NULL);
	}

	return queue;
}

/*xmq_queue_close - 关闭一个队列
 * @queue:          要关闭的队列结构指针
 * return:          成功返回0，失败返回-1
 * */
extern int xmq_queue_close(xmq_queue_t *queue)
{
	return_if_false((queue && queue->context), -1);
	xmq_context_t *ctx = queue->context;

	if (xmq_queue_is_active(ctx, queue->name)) {
		pthread_mutex_lock(&ctx->node_tree_lock);

		if (!list_empty(&queue->active_producers)) {
			pthread_mutex_unlock(&ctx->node_tree_lock);
			return -1;
		}

		if (!list_empty(&queue->active_consumers)) {
			pthread_mutex_unlock(&ctx->node_tree_lock);
			return -1;
		}

		list_del_all_entrys(&queue->dead_producers, xmq_producer_t, list);
		list_del_all_entrys(&queue->dead_consumers, xmq_consumer_t, list);

		pthread_mutex_destroy(&queue->wait_lock);
		pthread_cond_destroy(&queue->wait_cond);

		list_del(&queue->list);
		list_add_tail(&queue->list, &queue->context->dead_queues);

		pthread_mutex_unlock(&ctx->node_tree_lock);

		return 0;
	} else if (xmq_queue_exist(ctx, queue->name)) {
		x_printf(W, "queue [%s] is already closed", queue->name);
		return 0;
	}

	x_printf(E, "an error occured, queue point at address [%p]", queue);
	return -1;
}

/*xmq_producer_is_active - 队列中的某个生产者是否活动
 * @queue:                 队列指针
 * @producer_id:           生产者标志符字符串
 * return:                 活动返回生产者类型指针，否则返回NULL
 * */
extern xmq_producer_t *xmq_producer_is_active(xmq_queue_t *queue, const char *producer_id)
{
	xmq_producer_t *producer = NULL;

	return_if_false((queue && queue->context && producer_id && strlen(producer_id)), producer);

	xmq_context_t *ctx = queue->context;
	pthread_mutex_lock(&ctx->node_tree_lock);

	list_t *node;
	list_for_each(node, &queue->active_producers)
	{
		xmq_producer_t *p = container_of(node, xmq_producer_t, list);

		if (!strncmp(p->identity, producer_id, strlen(producer_id) + 1)) {
			producer = p;
			break;
		}
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);

	return producer;
}

/*xmq_producer_exist - 队列中的某个生产者是否存在
 * @queue:             队列指针
 * @producer_id:       生产者标志符字符串
 * return:             存在返回生产者类型指针，否则返回NULL
 * */
extern xmq_producer_t *xmq_producer_exist(xmq_queue_t *queue, const char *producer_id)
{
	xmq_producer_t *producer = NULL;

	return_if_false((queue && queue->context && producer_id && strlen(producer_id)), producer);

	if ((producer = xmq_producer_is_active(queue, producer_id))) {
		return producer;
	}

	xmq_context_t *ctx = queue->context;
	pthread_mutex_lock(&ctx->node_tree_lock);

	list_t *node;
	list_for_each(node, &queue->dead_producers)
	{
		xmq_producer_t *p = container_of(node, xmq_producer_t, list);

		if (!strncmp(p->identity, producer_id, strlen(producer_id) + 1)) {
			producer = p;
			break;
		}
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);

	return producer;
}

/*xmq_consumer_is_active - 队列中的某个消费者是否活动
 * @queue:                 队列指针
 * @consumer_id:           消费者标志符字符串
 * return:                 活动返回消费者类型指针，否则返回NULL
 * */
extern xmq_consumer_t *xmq_consumer_is_active(xmq_queue_t *queue, const char *consumer_id)
{
	xmq_consumer_t *consumer = NULL;

	return_if_false((queue && queue->context && consumer_id && strlen(consumer_id)), consumer);

	xmq_context_t *ctx = queue->context;
	pthread_mutex_lock(&ctx->node_tree_lock);

	list_t *node;
	list_for_each(node, &queue->active_consumers)
	{
		xmq_consumer_t *c = container_of(node, xmq_consumer_t, list);

		if (!strncmp(c->identity, consumer_id, strlen(consumer_id) + 1)) {
			consumer = c;
			break;
		}
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);

	return consumer;
}

/*xmq_consumer_exist - 队列中的某个消费者是否存在
 * @queue:             队列指针
 * @consumer_id:       消费者标志符字符串
 * return:             存在返回消费者类型指针，否则返回NULL
 * */
extern xmq_consumer_t *xmq_consumer_exist(xmq_queue_t *queue, const char *consumer_id)
{
	xmq_consumer_t *consumer = NULL;

	return_if_false((queue && queue->context && consumer_id && strlen(consumer_id)), consumer);

	if ((consumer = xmq_consumer_is_active(queue, consumer_id))) {
		return consumer;
	}

	xmq_context_t *ctx = queue->context;
	pthread_mutex_lock(&ctx->node_tree_lock);

	list_t *node;
	list_for_each(node, &queue->dead_consumers)
	{
		xmq_consumer_t *c = container_of(node, xmq_consumer_t, list);

		if (!strncmp(c->identity, consumer_id, strlen(consumer_id) + 1)) {
			consumer = c;
			break;
		}
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);

	return consumer;
}

/*xmq_register_as_producer - 向指定队列注册一个生产者，如果已存在则激活，如果没有则创建并激活
 * @queue:                   需要注册生产者的队列
 * @identity:                该生产者的标志字符串，在队列中唯一标志该生产者
 * return:                   生产者类型的结构体指针，失败返回NULL
 * */
extern xmq_producer_t *xmq_register_as_producer(xmq_queue_t *queue, const char *identity)
{
	return_if_false((queue && queue->context && identity && strlen(identity)), NULL);

	xmq_producer_t  *producer = NULL;
	xmq_context_t   *ctx = queue->context;

	if ((producer = xmq_producer_is_active(queue, identity))) {
		x_printf(W, "queue [%s] already has been registered", queue->name);
	} else if ((producer = xmq_producer_exist(queue, identity))) {
		pthread_mutex_lock(&ctx->node_tree_lock);

		list_del(&producer->list);
		list_add_tail(&producer->list, &queue->active_producers);

		pthread_mutex_unlock(&ctx->node_tree_lock);
	} else {
		pthread_mutex_lock(&ctx->node_tree_lock);

		producer = (xmq_producer_t *)calloc(1, sizeof(xmq_producer_t));

		if (producer) {
			strncpy(producer->identity, identity, sizeof(producer->identity) - 1);
			producer->owner_queue = queue;

			char *producers_key = get_key_of_producers(queue->name);

			if (!producers_key) {
				freeif(producer);
				return NULL;
			}

			char *old_producers =
				__write_back_append(queue->context, producers_key, producer->identity, ",");
			free(producers_key);

			if (old_producers == (char *)-1) {
				freeif(producer);
				return NULL;
			}

			freeif(old_producers);

			list_add_tail(&producer->list, &queue->active_producers);
			queue->producer_amount++;
		}

		pthread_mutex_unlock(&ctx->node_tree_lock);
	}

	return producer;
}

/*xmq_unregister_producer - 在指定队列中注销一个生产者，将其置为非激活状态，但绝不删除
 * @queue:                  需要为之注销的队列
 * @identity:               生产者标志符字符串
 * return:                  成功返回0，失败返回-1
 * */
extern int xmq_unregister_producer(xmq_queue_t *queue, const char *identity)
{
	return_if_false((queue && queue->context && identity && strlen(identity)), -1);
	xmq_context_t *ctx = queue->context;

	xmq_producer_t *producer;

	if ((producer = xmq_producer_is_active(queue, identity))) {
		pthread_mutex_lock(&ctx->node_tree_lock);

		list_del(&producer->list);
		list_add_tail(&producer->list, &queue->dead_producers);

		pthread_mutex_unlock(&ctx->node_tree_lock);
	}

	return 0;
}

/*检查一个生产者或消费者是否链入*/
#define __check_user(u)							\
	({  int __ret = 0;						\
	    if ((u) && (u)->owner_queue && (u)->owner_queue->context) {	\
		    __ret ++;						\
	    }								\
	    (int)__ret; })

/*__consumer_active - 将一个dead CONSUMERS激活
 * @consumer:         要被激活的消费者
 * return:            成功返回消费者类型指针，失败返回NULL
 * */
static xmq_consumer_t *__consumer_active(xmq_consumer_t *consumer)
{
	return_if_false(__check_user(consumer), NULL);

	xmq_queue_t     *queue = consumer->owner_queue;
	xmq_context_t   *ctx = queue->context;

	/*加载last_fetch_index，为空则置0*/
	char *fetchpos_key = get_key_of_fetchpos(queue->name, consumer->identity);
	return_if_false(fetchpos_key, NULL);

	int rc = __uint64_test_init(ctx, fetchpos_key, &consumer->last_fetch_index, (uint64_t)0);
	free(fetchpos_key);
	return_if_false((rc != -1), NULL);

	/*加载last_mark_index，为空则置0*/
	char *markpos_key = get_key_of_markpos(queue->name, consumer->identity);
	return_if_false(markpos_key, NULL);

	rc = __uint64_test_init(ctx, markpos_key, &consumer->last_mark_index, (uint64_t)0);
	free(markpos_key);
	return_if_false((rc != -1), NULL);

	list_del(&consumer->list);
	list_add_tail(&consumer->list, &queue->active_consumers);

	return consumer;
}

/*__consumer_new_active - 创建一个消费者并激活
 * @queue:                将要创建的消费者所属队列
 * @identity:             新的消费者标志字符串
 * return:                成功返回消费者类型指针，失败返回NULL
 * */
static xmq_consumer_t *__consumer_new_active(xmq_queue_t *queue, const char *identity)
{
	return_if_false((queue && queue->context && identity && strlen(identity)), NULL);

	xmq_context_t   *ctx = queue->context;
	xmq_consumer_t  *consumer = (xmq_consumer_t *)calloc(1, sizeof(xmq_consumer_t));

	if (consumer) {
		strncpy(consumer->identity, identity, sizeof(consumer->identity) - 1);
		consumer->owner_queue = queue;

		char *consumers_key = get_key_of_consumers(queue->name);

		if (!consumers_key) {
			goto dislike;
		}

		char *old_consumers =
			__write_back_append(ctx, consumers_key, consumer->identity, ",");
		free(consumers_key);

		if (old_consumers == (char *)-1) {
			goto dislike;
		}

		free(old_consumers);

		char *fetchpos_key = get_key_of_fetchpos(queue->name, consumer->identity);

		if (!fetchpos_key) {
			goto dislike;
		}

		int rc = __uint64_write(ctx, fetchpos_key, (uint64_t)0);
		free(fetchpos_key);

		if (-1 == rc) {
			goto dislike;
		}

		char *markpos_key = get_key_of_markpos(queue->name, consumer->identity);

		if (!markpos_key) {
			goto dislike;
		}

		rc = __uint64_write(ctx, markpos_key, (uint64_t)0);
		free(markpos_key);

		if (-1 == rc) {
			goto dislike;
		}

		list_add_tail(&consumer->list, &queue->active_consumers);
		consumer->slot = queue->consumer_amount;
		queue->consumer_amount++;
	}

	return consumer;

dislike:
	free(consumer);
	return NULL;
}

/*xmq_register_as_consumer - 向指定队列注册一个消费者，如果已存在则激活，如果没有则创建并激活
 * @queue:                   需要注册消费者的队列
 * @identity:                该消费者的标志字符串，在队列中唯一标志该消费者
 * return:                   消费者类型的结构体指针，失败返回NULL
 * */
extern xmq_consumer_t *xmq_register_as_consumer(xmq_queue_t *queue, const char *identity)
{
	return_if_false((queue && queue->context && identity && strlen(identity)), NULL);

	xmq_consumer_t  *consumer = NULL;
	xmq_context_t   *ctx = queue->context;

	if ((consumer = xmq_consumer_is_active(queue, identity))) {
		x_printf(W, "consumer [%s] already has been registered", identity);
	} else if ((consumer = xmq_consumer_exist(queue, identity))) {
		pthread_mutex_lock(&ctx->node_tree_lock);
		consumer = __consumer_active(consumer);
	} else {
		pthread_mutex_lock(&ctx->node_tree_lock);
		consumer = __consumer_new_active(queue, identity);
	}

	pthread_mutex_unlock(&ctx->node_tree_lock);
	return consumer;
}

/*xmq_unregister_consumer - 在指定队列中注销一个消费者，将其置为非激活状态，但绝不删除
 * @queue:                  需要为之注销的队列
 * @identity:               消费者标志符字符串
 * return:                  成功返回0，失败返回-1
 * */
extern int xmq_unregister_consumer(xmq_queue_t *queue, const char *identity)
{
	return_if_false((queue && queue->context && identity && strlen(identity)), -1);
	xmq_context_t *ctx = queue->context;

	xmq_consumer_t *consumer;

	if ((consumer = xmq_consumer_is_active(queue, identity))) {
		pthread_mutex_lock(&ctx->node_tree_lock);

		list_del(&consumer->list);
		list_add_tail(&consumer->list, &queue->dead_consumers);

		pthread_mutex_unlock(&ctx->node_tree_lock);
	}

	return 0;
}

/*queue_push_tail - 由生产者调用，向队列尾部追加一个消息
 * @producer:       生产者类型的结构体指针
 * @msg:            所要追加的消息
 * return:          成功返回0，失败返回-1
 * */
extern int queue_push_tail(xmq_producer_t *producer, xmq_msg_t *msg)
{
	return_if_false((__check_user(producer) && msg), -1);

	xmq_queue_t     *queue = producer->owner_queue;
	xmq_context_t   *ctx = queue->context;

	char *capacity_key = get_key_of_capacity(queue->name);
	return_if_false(capacity_key, -1);

	if (0 == queue->msg_capacity) {
		uint64_t        capacity;
		int             rc = __uint64_test_init(ctx, capacity_key, &capacity, (uint64_t)0);
		return_if_false((rc != -1), -1);

		queue->msg_capacity = capacity;
	}

	uint64_t index = queue->msg_capacity + 1;
	xmq_msg_set_index(msg, index);

	binary_entry_t key[2], value[2];
	set_bin_entry(key, &index, sizeof(index));
	set_bin_entry(value, msg, xmq_msg_total_size(msg));
	set_bin_entry(key + 1, capacity_key, strlen(capacity_key));
	set_bin_entry(value + 1, &index, sizeof(index));

	int res = ctx->put_KVs_cb(ctx->pvt, key, value, 2);
	free(capacity_key);

	if (0 == res) {
		if (queue->flag & F_QUEUE_BLOCK) {
			pthread_mutex_lock(&queue->wait_lock);
			queue->msg_capacity++;

			if (queue->waiters) {
				pthread_cond_broadcast(&queue->wait_cond);
			}

			pthread_mutex_unlock(&queue->wait_lock);
		}
	}

	return res;
}

/*__msg_fetch - 从队列指定位置读取一个消息
 * @consumer:   消费者类型的结构体指针
 * @index:      所要读取的消息的索引号，同时也表示当前队列读取位置
 * return:      返回消息结构指针，无此消息返回NULL，错误返回(xmq_msg_t *)-1
 * */
static xmq_msg_t *__msg_fetch(xmq_consumer_t *consumer, uint64_t index)
{
	return_if_false(__check_user(consumer), (xmq_msg_t *)-1);

	xmq_queue_t     *queue = consumer->owner_queue;
	xmq_context_t   *ctx = queue->context;

	return_if_false((index && index <= queue->msg_capacity), (xmq_msg_t *)-1);

	binary_entry_t key, value;
	set_bin_entry(&key, &index, sizeof(index));

	int res = ctx->get_KVs_cb(ctx->pvt, &key, &value, 1);
	return_if_false((res != -1), (xmq_msg_t *)-1);

	xmq_msg_t *msg = (xmq_msg_t *)value.data;

	if (xmq_msg_total_size(msg) != value.len) {
		xmq_msg_destroy(msg);
		return (xmq_msg_t *)-1;
	}

	return msg;
}

/*queue_fetch_nth - 由消费者调用，从队列指定位置读取一个消息，并更新读取位置
 * @consumer:       消费者类型的结构体指针
 * @index:          所要读取的消息的索引号，同时也表示当前队列读取位置
 * @millis:         有锁等待的毫秒数
 * return:          返回消息结构指针，无此消息返回NULL，错误返回(xmq_msg_t *)-1
 * */
extern xmq_msg_t *queue_fetch_nth(xmq_consumer_t *consumer, uint64_t index, unsigned int millis)
{
	return_if_false(__check_user(consumer), (xmq_msg_t *)-1);

	xmq_queue_t     *queue = consumer->owner_queue;
	xmq_context_t   *ctx = queue->context;

	if (queue->flag & F_QUEUE_BLOCK) {
		pthread_mutex_lock(&queue->wait_lock);

		while (index > queue->msg_capacity) {
			if (millis) {
				struct timeval  now;
				struct timespec timeout;

				gettimeofday(&now, NULL);

				if ((now.tv_usec + (suseconds_t)1000 * millis) >= (suseconds_t)1000000) {
					now.tv_sec += 1;
					now.tv_usec += ((suseconds_t)1000000 - (suseconds_t)1000 * millis);
				} else {
					now.tv_usec += (suseconds_t)1000 * millis;
				}

				timeout.tv_sec = now.tv_sec;
				timeout.tv_nsec = now.tv_usec * 1000;

				queue->waiters++;
				int res = pthread_cond_timedwait(&queue->wait_cond, &queue->wait_lock, &timeout);
				queue->waiters--;

				if (ETIMEDOUT == res) {
					pthread_mutex_unlock(&queue->wait_lock);
					return NULL;
				}
			} else {
				queue->waiters++;
				pthread_cond_wait(&queue->wait_cond, &queue->wait_lock);
				queue->waiters--;
			}
		}

		pthread_mutex_unlock(&queue->wait_lock);
	} else {
		return_if_false((index <= queue->msg_capacity), NULL);
	}

	xmq_msg_t *msg = __msg_fetch(consumer, index);

	if (msg) {
		char *fetchpos_key = get_key_of_fetchpos(queue->name, consumer->identity);

		if (!fetchpos_key) {
			xmq_msg_destroy(msg);
			return (xmq_msg_t *)-1;
		}

		int rc = __uint64_write(ctx, fetchpos_key, index);
		free(fetchpos_key);

		if (-1 == rc) {
			xmq_msg_destroy(msg);
			return (xmq_msg_t *)-1;
		}

		consumer->last_fetch_index = index;
	}

	return msg;
}

/*queue_fetch_next - 由消费者调用，从队列中读取下一个消息
 * @consumer:        消费者类型的结构体指针
 * @millis:          有锁等待的毫秒数
 * return:           返回消息结构指针，无此消息返回NULL，错误返回(xmq_msg_t *)-1
 * */
extern xmq_msg_t *queue_fetch_next(xmq_consumer_t *consumer, unsigned int millis)
{
	uint64_t index = consumer->last_fetch_index + 1;

	return queue_fetch_nth(consumer, index, millis);
}

/*queue_fetch_pos - 由消费者调用，获得队列当前读取位置
 * @consumer:       消费者类型的结构体指针
 * return:          最后一次读取到的消息的索引号，错误返回(uint64_t)-1
 * */
extern uint64_t queue_fetch_pos(xmq_consumer_t *consumer)
{
	return_if_false(__check_user(consumer), (uint64_t)-1);
	return consumer->last_fetch_index;
}

/*queue_mark_nth - 由消费者调用，标记队列指定位置
 * @consumer:      消费者类型的结构体指针
 * @index:         所要标记的消息的索引号
 * return:         返回之前的标记，其他返回(uint64_t)-1
 * */
extern uint64_t queue_mark_nth(xmq_consumer_t *consumer, uint64_t index)
{
	return_if_false(__check_user(consumer), (uint64_t)-1);

	xmq_queue_t     *queue = consumer->owner_queue;
	xmq_context_t   *ctx = queue->context;

	return_if_false((index <= queue->msg_capacity), (uint64_t)-1);

	char *markpos_key = get_key_of_markpos(queue->name, consumer->identity);
	return_if_false(markpos_key, (uint64_t)-1);

	binary_entry_t key, value;

	set_bin_entry(&key, markpos_key, strlen(markpos_key));
	set_bin_entry(&value, &index, sizeof(index));

	int res = ctx->put_KVs_cb(ctx->pvt, &key, &value, 1);
	free(markpos_key);
	return_if_false((0 == res), (uint64_t)-1);

	return consumer->last_mark_index;
}

/*queue_mark_next - 由消费者调用，移动标记到队列下一个消息
 * @consumer:       消费者类型的结构体指针
 * return:          返回之前的标记，其他返回(uint64_t)-1
 * */
extern uint64_t queue_mark_next(xmq_consumer_t *consumer)
{
	uint64_t index = consumer->last_mark_index + 1;

	return queue_mark_nth(consumer, index);
}

/*queue_mark_pos - 由消费者调用，获得队列当前标记位置
 * @consumer:      消费者类型的结构体指针
 * return:         最后一次成功标记的消息的索引号
 * */
extern uint64_t queue_mark_pos(xmq_consumer_t *consumer)
{
	return_if_false(__check_user(consumer), (uint64_t)0);
	return consumer->last_mark_index;
}

