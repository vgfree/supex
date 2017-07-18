#include <assert.h>
#include <string.h>

#include "comm_message_operator.h"
// #include "loger.h"
#include "comm_api.h"


void init_msg(struct comm_message *msg)
{
	assert(msg);
	msg->package.packages = 1;
	msg->ptype = PUSH_METHOD;
	commmsg_make(msg, MAX_MSG_SIZE * sizeof(char));
}

void destroy_msg(struct comm_message *msg)
{
	assert(msg);
	commmsg_free(msg);
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
	assert(msg);

	if ((index >= msg->package.frames) || (index < 0)) {
		//    error("index:%d > max frames:%d.", index, msg->package.frames);
		return NULL;
	}

	*size = msg->package.frame_size[index];
	return msg->package.raw_data.str + msg->package.frame_offset[index];
}

int set_msg_frame(int index, struct comm_message *msg, int size, char *frame)
{
	// 默认msg->content 已经malloc 了足够大的空间。
	assert(msg && frame);

	if ((index > msg->package.frames) || (index < 0)) {
		//    error("index:%d > max frames:%d.", index, msg->package.frames);
		return -1;
	}

	if (index == msg->package.frames) {
		memcpy(msg->package.raw_data.str + msg->package.raw_data.len, frame, size);
		msg->package.frames++;
		msg->package.frame_size[index] = size;
		msg->package.frame_offset[index] = msg->package.raw_data.len;
		msg->package.raw_data.len += size;
	} else {
		memmove(msg->package.raw_data.str + msg->package.frame_offset[index] + size,
			msg->package.raw_data.str + msg->package.frame_offset[index],
			msg->package.raw_data.len - msg->package.frame_offset[index]);
		memcpy(msg->package.raw_data.str + msg->package.frame_offset[index],
			frame, size);

		for (int i = msg->package.frames; i > index; i--) {
			msg->package.frame_size[i] = msg->package.frame_size[i - 1];
			msg->package.frame_offset[i] = msg->package.frame_offset[i - 1] + size;
		}

		msg->package.frames++;
		msg->package.frame_size[index] = size;
		msg->package.raw_data.len += size;
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
	msg->package.raw_data.len -= rmsz;
	memmove(msg->package.raw_data.str, msg->package.raw_data.str + rmsz,
		msg->package.raw_data.len);

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

