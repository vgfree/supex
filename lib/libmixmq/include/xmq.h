#pragma once

#include <stdint.h>
#include <pthread.h>

#include "xmq_msg.h"
#include "xmq_list.h"

/****************************************************
 *         生产者、消费者                           *
 ***************************************************/

typedef struct xmq_queue_struct xmq_queue_t;
typedef struct xmq_context_struct xmq_context_t;

/*生产者类型*/
typedef struct xmq_producer_struct
{
	char            identity[256];	// 生产者标志字符串

	list_t          list;		// 用于链接兄弟生产者，如果有的话
	xmq_queue_t     *owner_queue;	// 指向该生产者所属的队列
} xmq_producer_t;

/*消费者类型*/
typedef struct xmq_consumer_struct
{
	char            identity[256];		// 消费者标志字符串

	list_t          list;			// 用于链接兄弟消费者，如果有的话
	unsigned int    slot;			// 槽位号，从0开始，加载的时候被初始化，用于寻址状态位

	uint64_t        last_fetch_index;	// 消费者上次获取的消息索引号
	uint64_t        last_mark_index;	// 消费者上次记录状态的消息索引号

	xmq_queue_t     *owner_queue;		// 指向该消费者所属的队列
} xmq_consumer_t;

/*xmq_producer_is_active - 队列中的某个生产者是否活动
 * @queue:                 队列指针
 * @producer_id:           生产者标志符字符串
 * return:                 活动返回生产者类型指针，否则返回NULL
 * */
extern xmq_producer_t *xmq_producer_is_active(xmq_queue_t *queue, const char *producer_id);

/*xmq_producer_exist - 队列中的某个生产者是否存在
 * @queue:             队列指针
 * @producer_id:       生产者标志符字符串
 * return:             存在返回生产者类型指针，否则返回NULL
 * */
extern xmq_producer_t *xmq_producer_exist(xmq_queue_t *queue, const char *producer_id);

/*xmq_consumer_is_active - 队列中的某个消费者是否活动
 * @queue:                 队列指针
 * @consumer_id:           消费者标志符字符串
 * return:                 活动返回消费者类型指针，否则返回NULL
 * */
extern xmq_consumer_t *xmq_consumer_is_active(xmq_queue_t *queue, const char *consumer_id);

/*xmq_consumer_exist - 队列中的某个消费者是否存在
 * @queue:             队列指针
 * @consumer_id:       消费者标志符字符串
 * return:             存在返回消费者类型指针，否则返回NULL
 * */
extern xmq_consumer_t *xmq_consumer_exist(xmq_queue_t *queue, const char *consumer_id);

/*xmq_register_as_producer - 向指定队列注册一个生产者，如果已存在则激活，如果没有则创建并激活
 * @queue:                   需要注册生产者的队列
 * @identity:                该生产者的标志字符串，在队列中唯一标志该生产者
 * return:                   生产者类型的结构体指针，失败返回NULL
 * */
extern xmq_producer_t *xmq_register_as_producer(xmq_queue_t *queue, const char *identity);

/*xmq_register_as_producer - 向指定队列注册一个消费者，如果已存在则激活，如果没有则创建并激活
 * @queue:                   需要注册消费者的队列
 * @identity:                该消费者的标志字符串，在队列中唯一标志该消费者
 * return:                   消费者类型的结构体指针，失败返回NULL
 * */
extern xmq_consumer_t *xmq_register_as_consumer(xmq_queue_t *queue, const char *identity);

/*xmq_unregister_producer - 在指定队列中注销一个生产者，将其置为非激活状态，但绝不删除
 * @queue:                  需要为之注销的队列
 * @identity:               生产者标志符字符串
 * return:                  成功返回0，失败返回-1
 * */
extern int xmq_unregister_producer(xmq_queue_t *queue, const char *identity);

/*xmq_unregister_consumer - 在指定队列中注销一个消费者，将其置为非激活状态，但绝不删除
 * @queue:                  需要为之注销的队列
 * @identity:               消费者标志符字符串
 * return:                  成功返回0，失败返回-1
 * */
extern int xmq_unregister_consumer(xmq_queue_t *queue, const char *identity);

/****************************************************
 *         上下文、队列                             *
 ***************************************************/

/*二进制数据块*/
typedef struct binary_entry_struct
{
	void            *data;
	unsigned int    len;
} binary_entry_t;

#define set_bin_entry(bin, ptr, size)		   \
	do {					   \
		(bin)->data = (void *)(ptr);	   \
		(bin)->len = (unsigned int)(size); \
	} while (0)

#define release_bin_entry(bin)	   \
	do {			   \
		free((bin)->data); \
		(bin)->len = 0;	   \
	} while (0)

#define bin_dup_string(b) strndup((b)->data, (b)->len)

/*以下函数类型由用户实现，函数实现可能因媒介不同(如leveldb、redis等)而不同*/
typedef int (put_KVs_fn)(void *context_pvt, binary_entry_t *keys, binary_entry_t *values, unsigned int pairs);
typedef int (get_KVs_fn)(void *context_pvt, binary_entry_t *keys, binary_entry_t *values, unsigned int count);
typedef int (pvt_free_fn)(void *pvt);

/*上下文结构类型*/
struct xmq_context_struct
{
	void            *pvt;		// 由用户提供的私用数据
	pvt_free_fn     *pvt_free_cb;	// 如上，释放用户私有数据context_pvt 的回调

	put_KVs_fn      *put_KVs_cb;	// 如上，由用户实现的存储键值对回调
	get_KVs_fn      *get_KVs_cb;	// 如上，由用户实现的获取键值对回调

	list_t          active_queues;	// 当前已打开的队列
	list_t          dead_queues;	// 已经存在，但当前未被打开的队列

	int             queue_amount;	// 当前队列数量

	pthread_mutex_t node_tree_lock;	// 所有链表操作互斥锁
};					// xmq_context_t

#define    F_QUEUE_NONE         0x0
#define    F_QUEUE_BLOCK        0x1

/*队列结构类型*/
struct xmq_queue_struct
{
	char            name[256];		// 队列名
	list_t          list;			// 链接所有其他队列，如果有的话
	uint64_t        id;			// 队列序号从0开始

	uint64_t        msg_capacity;		// 当前队列消息总量，应当等于最后一个入队消息的索引号
	unsigned int    flag;			// 标识符
	unsigned int    waiters;		// 阻塞模式下消费者处于等待状态的个数
	pthread_mutex_t wait_lock;		// 用于陷入等待的互斥锁
	pthread_cond_t  wait_cond;		// 用于陷入等待的条件变量

	xmq_context_t   *context;		//上下文变量，包含基于不同媒介(如leveldb, redis)，用以
						// 实现队列的所需参量和接口

	list_t          active_producers;	// 活动生产者，自本次程序启动以来，已经完成注册
	list_t          active_consumers;	// 活动消费者，自本次程序启动以来，已经完成注册

	list_t          dead_producers;		// 非活动生产者
	list_t          dead_consumers;		// 非活动消费者
	// 1.指曾经注册为生产者/消费者，但是在本次程序启动后还尚未注册
	// 2.本次程序启动后已经完成注册，但后来又被注销

	int             producer_amount;	// 当前生产者计数
	int             consumer_amount;	// 当前消费者计数
};

/*xmq_context_new - 动态内存分配一个上下文
 * return:          成功返回上下文结构类型指针，失败返回NULL
 * */
extern xmq_context_t *xmq_context_new(void);

/*xmq_context_init - 使用存储媒介相关的私有数据、回调函数初始化上下文结构，并从存储媒介中载入队列相关信息
 * @ctx:             需要初始化的上下文
 * @pvt:             媒介相关用户私用数据
 * @pvt_free:        释放用户私有数据回调
 * return:           成功返回0，失败返回-1
 * */
extern int xmq_context_init(xmq_context_t *ctx, void *pvt, pvt_free_fn *pvt_free,
	put_KVs_fn put_kvs, get_KVs_fn get_kvs);

/*xmq_context_term - 释放上下文，队列必须全部closed
 *                   如果当前还有活动的队列将会导致失败
 * @ctx:             需要释放的上下文结构指针
 * return:           成功返回0，失败返回-1
 * */
extern int xmq_context_term(xmq_context_t *ctx);

/*xmq_queue_is_active - 队列是否活动
 * @ctx:                上下文结构指针
 * @qname:              查看是否活动的队列名
 * return:              活动返回队列指针，否则返回NULL
 * */
extern xmq_queue_t *xmq_queue_is_active(xmq_context_t *ctx, const char *qname);

/*xmq_queue_exist - 队列是否存在
 * @ctx:            上下文结构指针
 * @qname:          查看是否存在的队列名
 * return:          存在返回队列地址，否则返回NULL
 * */
extern xmq_queue_t *xmq_queue_exist(xmq_context_t *ctx, const char *qname);

/*xmq_queue_open - 打开一个队列，如果不存在则创建
 * @ctx:           上下文结构指针
 * @queue:         队列名，唯一标识一个队列
 * @flag:          标识符，当前主要标记阻塞非阻塞
 * return:         队列结构类型，失败返回NULL
 * */
extern xmq_queue_t *xmq_queue_open(xmq_context_t *ctx, const char *name, unsigned int flag);

/*xmq_queue_close - 关闭一个队列
 * @queue:          要关闭的队列结构指针
 * return:          成功返回0，失败返回-1
 * */
extern int xmq_queue_close(xmq_queue_t *queue);

/****************************************************
 *         队列读写API                              *
 ***************************************************/

/*queue_push_tail - 由生产者调用，向队列尾部追加一个消息
 * @producer:       生产者类型的结构体指针
 * @msg:            所要追加的消息
 * return:          成功返回0，失败返回-1
 * */
extern int queue_push_tail(xmq_producer_t *producer, xmq_msg_t *msg);

/*queue_fetch_nth - 由消费者调用，从队列指定位置读取一个消息
 * @consumer:       消费者类型的结构体指针
 * @index:          所要读取的消息的索引号，同时也表示当前队列读取位置
 * @millis:         有锁等待的毫秒数
 * return:          返回消息结构指针，无此消息返回NULL，错误返回(xmq_msg_t *)-1
 * */
extern xmq_msg_t *queue_fetch_nth(xmq_consumer_t *consumer, uint64_t index, unsigned int millis);

/*queue_fetch_next - 由消费者调用，从队列中读取下一个消息
 * @consumer:        消费者类型的结构体指针
 * @millis:          有锁等待的毫秒数
 * return:           返回消息结构指针，无此消息返回NULL，错误返回(xmq_msg_t *)-1
 * */
extern xmq_msg_t *queue_fetch_next(xmq_consumer_t *consumer, unsigned int millis);

/*queue_fetch_pos - 由消费者调用，获得队列当前读取位置
 * @consumer:       消费者类型的结构体指针
 * return:          返回消息结构指针，无此消息返回NULL，错误返回(xmq_msg_t *)-1
 * */
extern uint64_t queue_fetch_pos(xmq_consumer_t *consumer);

/*queue_mark_nth - 由消费者调用，标记队列指定位置
 * @consumer:      消费者类型的结构体指针
 * @index:         所要标记的消息的索引号
 * return:         返回之前的标记，其他返回(uint64_t)-1
 * */
extern uint64_t queue_mark_nth(xmq_consumer_t *consumer, uint64_t index);

/*queue_mark_next - 由消费者调用，标记队列下一个消息状态
 * @consumer:       消费者类型的结构体指针
 * return:         返回之前的标记，其他返回(uint64_t)-1
 * */
extern uint64_t queue_mark_next(xmq_consumer_t *consumer);

/*queue_mark_pos - 由消费者调用，获得队列当前读取位置
 * @consumer:      消费者类型的结构体指针
 * return:         最后一次成功标记的消息的索引号
 * */
extern uint64_t queue_mark_pos(xmq_consumer_t *consumer);

