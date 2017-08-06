#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "comm_utils.h"
#include "comm_message.h"

struct comm_message *commmsg_make(struct comm_message *message, size_t cap)
{
	if (message) {
		bzero(message, sizeof(*message));
		message->canfree = false;
	} else {
		message = calloc(1, sizeof(struct comm_message));
		message->canfree = true;
	}

	commsds_make(&message->package.raw_data, cap);
	return message;
}

void commmsg_copy(struct comm_message *dstmsg, struct comm_message *srcmsg)
{
	assert(dstmsg && srcmsg);
	dstmsg->fd = srcmsg->fd;
	dstmsg->flags = srcmsg->flags;
	dstmsg->ptype = srcmsg->ptype;

	commsds_copy(&dstmsg->package.raw_data, &srcmsg->package.raw_data);
	memcpy((char *)&dstmsg->package + sizeof(struct comm_sds),
		(char *)&srcmsg->package + sizeof(struct comm_sds),
		sizeof(srcmsg->package) - sizeof(struct comm_sds));
}

void commmsg_free(struct comm_message *message)
{
	commsds_free(&message->package.raw_data);

	if (message->canfree) {
		free(message);
	}
}

/* 检测包的信息是否设置正确 */
bool commmsg_check(const struct comm_message *message)
{
	assert(message);
	int     index = 0;
	int     offset = 0;	/* 数据的总大小 */
	int     pckidx = 0;	/* 包的索引 */
	int     frmidx = 0;	/* 帧的索引 */
	int     frames = 0;	/* 帧的总数 */

	int dtsize = message->package.raw_data.len;

	if (unlikely(message->package.packages < 1)) {
		loger("wrong packages in comm_message structure, packages:%d", message->package.packages);
		return false;
	}

	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		if ((message->package.frames_of_package[pckidx] < 1) || (message->package.frames_of_package[pckidx] > message->package.frames)) {
			loger("wrong sum of frames in frames_of_pack of comm_message structure, frames:%d, index:%d\n", message->package.frames_of_package[pckidx], pckidx);
			return false;
		}

		for (frmidx = 0; frmidx < message->package.frames_of_package[pckidx]; frmidx++, index++) {
			if (unlikely((offset > dtsize) || (message->package.frame_size[index] > dtsize))) {
				loger("wrong frame_size in comm_message structure, frame_size:%d index:%d\n", message->package.frame_size[index], index);
				return false;
			}

			if (unlikely(message->package.frame_offset[index] != offset)) {
				loger("wrong frame_offset in comm_package, frame_offset:%d index:%d\n", message->package.frame_offset[index], index);
				return false;
			}

			offset += message->package.frame_size[index];
			frames++;
		}
	}

	if (unlikely(frames != message->package.frames)) {
		loger("wrong sum of frames in comm_message structure, frames:%d\n", message->package.frames);
		return false;
	}

	if (unlikely(offset != dtsize)) {
		loger("wrong sum of datasize in comm_message structure, datasize:%d\n", dtsize);
		return false;
	}

	return true;
}


void commmsg_sets(struct comm_message *message, int fd, int flags, int ptype)
{
	assert(message);
	message->fd = fd;
	message->flags = flags;
	message->ptype = ptype;
}

void commmsg_gets(struct comm_message *message, int *fd, int *flags, int *ptype)
{
	assert(message);
	if (fd) {
		*fd = message->fd;
	}
	if (flags) {
		*flags = message->flags;
	}
	if (ptype) {
		*ptype = message->ptype;
	}
}

char *commmsg_frame_get(struct comm_message *msg, int index, int *size)
{
        assert(msg);

        if ((index >= msg->package.frames) || (index < 0)) {
                printf("index:%d > max frames:%d.", index, msg->package.frames);
                return NULL;
        }

        *size = commmsg_frame_size(msg, index);
        return commmsg_frame_addr(msg, index);
}
