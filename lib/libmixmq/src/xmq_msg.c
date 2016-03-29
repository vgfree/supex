#include <stdlib.h>
#include <string.h>

#include "xmq_msg.h"

/*xmq_msg_new_size - 创建一个消息
 * @size:            消息容量大小
 * return:           成功返回消息类型指针，失败返回NULL
 * */
extern xmq_msg_t *xmq_msg_new_size(size_t size)
{
	xmq_msg_t *msg = (xmq_msg_t *)calloc(1, size + MSG_HEAD_SIZE);

	if (msg) {
		msg->len = size;
	}

	return msg;
}

/*xmq_msg_new_data - 创建一个消息
 * @data:            写入消息中的数据头指针
 * @size:            要写入的数据大小
 * return:           成功返回消息类型指针，失败返回NULL
 * */
extern xmq_msg_t *xmq_msg_new_data(const void *data, size_t size)
{
	if ((NULL == data) || (0 == size)) {
		return NULL;
	}

	xmq_msg_t *msg = xmq_msg_new_size(size);

	if (msg) {
		memcpy(xmq_msg_data(msg), data, size);
	}

	return msg;
}

/*xmq_msg_dup - 复制一个消息
 * @src:        源消息地址
 * return:      成功返回消息类型指针，失败返回NULL
 * */
extern xmq_msg_t *xmq_msg_dup(const xmq_msg_t *src)
{
	if (!src) {
		return NULL;
	}

	return xmq_msg_new_data(xmq_msg_data(src), xmq_msg_size(src));
}

/*xmq_msg_set_index - 设置消息的索引号
 * @msg:              需要设置消息索引号的消息
 * @index:            消息索引号
 * return:            消息之前的索引号
 * */
extern uint64_t xmq_msg_set_index(xmq_msg_t *msg, uint64_t index)
{
	if (!msg) {
		return (uint64_t)0;
	}

	uint64_t old = xmq_msg_index(msg);
	msg->index = index;

	return old;
}

