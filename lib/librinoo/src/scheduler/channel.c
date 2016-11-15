/**
 * @file   channel.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2015
 * @date   Mon Dec  7 20:53:38 2015
 *
 * @brief Channel functions
 *
 *
 */

#include "rinoo/scheduler/module.h"

/**
 * Create a new channel.
 *
 * @param sched Pointer to the scheduler to user.
 *
 * @return Pointer to the new channel, or NULL if an error occurs.
 */
t_channel *rinoo_channel(t_sched *sched)
{
	t_channel *channel;

	channel = calloc(1, sizeof(*channel));
	if (channel == NULL) {
		return NULL;
	}
	channel->sched = sched;
	return channel;
}

/**
 * Destroy a channel.
 *
 * @param channel Channel to destroy.
 */
void rinoo_channel_destroy(t_channel *channel)
{
	free(channel);
}

void *rinoo_channel_get(t_channel *channel)
{
	void *result;
	t_task *task;
	t_sched *sched;

	sched = rinoo_sched_self();
	if (channel->sched != sched) {
		return NULL;
	}
	if (channel->buf == NULL) {
		channel->task = rinoo_task_self();
		rinoo_task_release(sched);
		if (channel->buf == NULL) {
			return NULL;
		}
	}
	result = channel->buf;
	task = channel->task;
	channel->buf = NULL;
	channel->size = 0;
	channel->task = NULL;
	rinoo_task_schedule(task, NULL);
	return result;
}

int rinoo_channel_put(t_channel *channel, void *ptr)
{
	if (rinoo_channel_write(channel, ptr, 0) < 0) {
		return -1;
	}
	return 0;
}

/**
 * Read from a channel. This is blocking.
 *
 * @param channel Channel to read.
 * @param dest Pointer to memory where to store result.
 * @param size Size of memory read.
 *
 * @return number of bytes read on success, or -1 if an error occurs.
 */
int rinoo_channel_read(t_channel *channel, void *dest, size_t size)
{
	t_task *task;
	t_sched *sched;

	sched = rinoo_sched_self();
	if (channel->sched != sched) {
		return -1;
	}
	if (channel->buf == NULL) {
		channel->task = rinoo_task_self();
		rinoo_task_release(sched);
		if (channel->buf == NULL) {
			return -1;
		}
	}
	if (size > channel->size) {
		size = channel->size;
	}
	memcpy(dest, channel->buf, size);
	channel->size -= size;
	if (channel->size > 0) {
		channel->buf += size;
	} else {
		task = channel->task;
		channel->buf = NULL;
		channel->size = 0;
		channel->task = NULL;
		rinoo_task_schedule(task, NULL);
	}
	return size;
}

/**
 * Write to a channel. This is blocking.
 *
 * @param channel Channel to read.
 * @param buf Buffer to write.
 * @param size Size of buf.
 *
 * @return Number of bytes written on success, or -1 if an error occurs.
 */
int rinoo_channel_write(t_channel *channel, void *buf, size_t size)
{
	t_task *task;
	t_sched *sched;

	sched = rinoo_sched_self();
	if (channel->sched != sched) {
		return -1;
	}
	channel->buf = buf;
	channel->size = size;
	task = channel->task;
	if (task != NULL) {
		rinoo_task_schedule(task, NULL);
	}
	channel->task = rinoo_task_self();
	rinoo_task_release(sched);
	return size;
}
