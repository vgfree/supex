#ifndef RINOO_SCHEDULER_CHANNEL_H_
#define RINOO_SCHEDULER_CHANNEL_H

typedef struct s_channel {
	void *buf;
	size_t size;
	t_task *task;
	t_sched *sched;
} t_channel;

t_channel *rinoo_channel(t_sched *sched);
void rinoo_channel_destroy(t_channel *channel);
void *rinoo_channel_get(t_channel *channel);
int rinoo_channel_put(t_channel *channel, void *ptr);
int rinoo_channel_read(t_channel *channel, void *dest, size_t size);
int rinoo_channel_write(t_channel *channel, void *buf, size_t size);

#endif /* !RINOO_SCHEDULER_CHANNEL_H_ */
