#include <assert.h>

#include "comm_message_operator.h"
// #include "loger.h"


void init_msg(struct comm_message *msg)
{
	assert(msg);
	msg->package.packages = 1;
	msg->socket_type = -1;
	msg->content = (char *)malloc(MAX_MSG_SIZE * sizeof(char));
}

void destroy_msg(struct comm_message *msg)
{
	assert(msg && msg->content);
	free(msg->content);
}

int get_msg_fd(struct comm_message *msg)
{
	assert(msg);
	return msg->fd;
}

void set_msg_fd(struct comm_message *msg, int fd)
{
	assert(msg);
	msg->fd = fd;
}

char *get_msg_frame(int index, struct comm_message *msg, int *size)
{
	assert(msg && msg->content);

	if ((index >= msg->package.frames) || (index < 0)) {
		//    error("index:%d > max frames:%d.", index, msg->package.frames);
		return NULL;
	}

	*size = msg->package.frame_size[index];
	return msg->content + msg->package.frame_offset[index];
}

int set_msg_frame(int index, struct comm_message *msg, int size, char *frame)
{
	// 默认msg->content 已经malloc 了足够大的空间。
	assert(msg && msg->content && frame);

	if ((index > msg->package.frames) || (index < 0)) {
		//    error("index:%d > max frames:%d.", index, msg->package.frames);
		return -1;
	}

	if (index == msg->package.frames) {
		memcpy(msg->content + msg->package.dsize, frame, size);
		msg->package.frames++;
		msg->package.frame_size[index] = size;
		msg->package.frame_offset[index] = msg->package.dsize;
		msg->package.dsize += size;
	} else {
		memmove(msg->content + msg->package.frame_offset[index] + size,
			msg->content + msg->package.frame_offset[index],
			msg->package.dsize - msg->package.frame_offset[index]);
		memcpy(msg->content + msg->package.frame_offset[index],
			frame, size);

		for (int i = msg->package.frames; i > index; i--) {
			msg->package.frame_size[i] = msg->package.frame_size[i - 1];
			msg->package.frame_offset[i] = msg->package.frame_offset[i - 1] + size;
		}

		msg->package.frames++;
		msg->package.frame_size[index] = size;
		msg->package.dsize += size;
	}

	msg->package.frames_of_package[0] = msg->package.frames;
	return 0;
}

int remove_first_nframe(int nframe, struct comm_message *msg)
{
	if (nframe > msg->package.frames) {
		//    error("nframe:%d > msg->package.frames:%d.", nframe, msg->package.frames);
	} else if (nframe == msg->package.frames) {
		memset(&msg->package, 0, sizeof(msg->package));
		msg->package.packages = 1;
		return 0;
	}

	int rmsz = msg->package.frame_offset[nframe];
	msg->package.dsize -= rmsz;
	memmove(msg->content, msg->content + rmsz,
		msg->package.dsize);

	int i = 0;

	for (i = 0; (i + nframe) < msg->package.frames; i++) {
		msg->package.frame_size[i] = msg->package.frame_size[nframe + i];
		msg->package.frame_offset[i] = msg->package.frame_offset[nframe + i] - rmsz;
	}

	msg->package.frames -= nframe;
	msg->package.frames_of_package[0] -= nframe;
	return rmsz;
}

int get_max_msg_frame(struct comm_message *msg)
{
	assert(msg);
	return msg->package.frames;
}

int get_frame_size(int index, struct comm_message *msg)
{
	assert(msg);
	return msg->package.frame_size[index];
}

