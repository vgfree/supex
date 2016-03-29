#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

/****************************************************
 *         队列消息                                 *
 ***************************************************/

/*消息头*/
typedef struct _xmq_msg_t
{
	size_t  len;
	char    data[0];	// 消息数据首地址
} xmq_msg_t;

/*消息头长度*/
#define xmq_msg_head_size (sizeof(xmq_msg_t))

/*xmq_msg_new_size - 创建一个消息
 * @size:            消息容量大小
 * return:           成功返回消息类型指针，失败返回NULL
 * */
extern xmq_msg_t *xmq_msg_new_size(size_t size);

/*xmq_msg_new_data - 创建一个消息
 * @data:            写入消息中的数据头指针
 * @size:            要写入的数据大小
 * return:           成功返回消息类型指针，失败返回NULL
 * */
extern xmq_msg_t *xmq_msg_new_data(const void *data, size_t size);

/*xmq_msg_dup - 复制一个消息
 * @src:        源消息地址
 * return:      成功返回消息类型指针，失败返回NULL
 * */
extern xmq_msg_t *xmq_msg_dup(const xmq_msg_t *src);

/*xmq_msg_data - 取得消息中消息体的数据
 * @msg:         消息指针
 * return:       消息体地址
 * */
#define xmq_msg_data(msgptr)            ((void *)((msgptr)->data))

/*xmq_msg_size - 取得消息中数据字节数
 * @msg:         消息指针
 * return:       消息中数据的字节数
 * */
#define xmq_msg_size(msgptr)            ((size_t)((msgptr)->len))

/*xmq_msg_total_size - 取得整个消息头+消息体大小
 * @msg:               消息指针
 * return:             整个消息头+消息体字节数
 * */
#define xmq_msg_total_size(msgptr)      ((size_t)(xmq_msg_head_size + (msgptr)->len))

/*xmq_msg_destroy - 释放消息
 * @msg:            需要释放的消息
 * return:          无
 * */
static inline void xmq_msg_destroy(xmq_msg_t *msg)
{
	if (msg) {
		free(msg);
	}
}

