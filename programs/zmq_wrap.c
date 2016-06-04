//
//  zmq_wrap.c
//  supex
//
//  Created by 周凯 on 15/12/9.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <zmq.h>
#include "zmq_wrap.h"

int get_zmqopt(void *skt, int opt)
{
	assert(skt);
	int     result = 0;
	int     rc = 0;
	size_t  optlen = sizeof(result);

	rc = zmq_getsockopt(skt, opt, &result, &optlen);
	RAISE_SYS_ERROR(rc);

	return result;
}

void set_zmqopt(void *skt, int opt, int value)
{
	assert(skt);
	int     flag = 0;
	size_t  vsize = sizeof(value);

	flag = zmq_setsockopt(skt, opt, &value, vsize);
	RAISE_SYS_ERROR(flag);
}

bool check_zmqwrite(void *skt, int to)
{
	assert(skt);
	int             flag = 0;
	zmq_pollitem_t  pitem[1] = { { .fd = -1 } };
	pitem[0].events = ZMQ_POLLOUT;
	pitem[0].socket = skt;

	flag = zmq_poll(pitem, DIM(pitem), to < 0 ? UINT32_MAX : to);

	if (likely(flag == 1)) {
		if (likely(pitem[0].revents & ZMQ_POLLOUT)) {
			return true;
		}
	} else if (unlikely(flag < 0)) {
		if (unlikely(errno != EINTR)) {
			RAISE(EXCEPT_SYS);
		}
	}

	return false;
}

bool check_zmqread(void *skt, int to)
{
	assert(skt);
	int             flag = 0;
	zmq_pollitem_t  pitem[1] = { { .fd = -1 } };
	pitem[0].events = ZMQ_POLLIN;
	pitem[0].socket = skt;

	flag = zmq_poll(pitem, DIM(pitem), to < 0 ? UINT32_MAX : to);

	if (likely(flag == 1)) {
		if (likely(pitem[0].revents & ZMQ_POLLIN)) {
			return true;
		}
	} else if (unlikely(flag < 0)) {
		if (unlikely(errno != EINTR)) {
			RAISE(EXCEPT_SYS);
		}
	}

	return false;
}

struct zmqframe *zmqframe_new(int size)
{
	return_val_if_fail(size >= 0, NULL);
	const int       base = (int)offsetof(struct zmqframe, data);
	struct zmqframe *frame = NULL;

	frame = malloc(size + base);

	if (unlikely(!frame)) {
		errno = ENOMEM;
		return NULL;
	}

	memset(frame, 0, base);
	frame->offset = base;
	frame->size = size + base;
	frame->magic = 0;
	frame->frames = 0;
	return frame;
}

struct zmqframe *zmqframe_resize(struct zmqframe **frameptr, int addsize)
{
	struct zmqframe *old = NULL;
	struct zmqframe *frame = NULL;

	assert(frameptr);

	old = *frameptr;

	if (unlikely(!old)) {
		frame = zmqframe_new(addsize);
	} else if (likely(addsize > 0)) {
		int remain = frame->size - frame->offset;

		if (likely(remain >= addsize)) {
			return old;
		} else {
			int size = addsize - remain;
			size += PKG_INCREMENT;
			size = ADJUST_SIZE(size + frame->offset, 8);

			frame = realloc(old, size);

			if (unlikely(!frame)) {
				size = addsize - remain;
				size += frame->offset;
				frame = realloc(old, size);

				if (unlikely(!frame)) {
					errno = ENOMEM;
					return NULL;
				}
			}

			frame->size = size;
		}
	} else if (likely(addsize < 0)) {
		const int       base = (int)offsetof(struct zmqframe, data);
		int             size = old->size + addsize;
		size = MAX(size, base);
		frame = realloc(old, size);

		if (unlikely(!frame)) {
			errno = ENOMEM;
			return NULL;
		} else {
			frame->size = size;

			if (frame->offset > size) {
				/*裁剪有效的数据*/
				x_printf(W, "cut valid data.");
				int     i = 0;
				int     valid = size - base;
				frame->offset = size;

				for (i = 0; valid > 0 && i < frame->frames; i++) {
					/*判断帧长度数组中的数据是否有效*/
					if (likely(frame->frame[i] > 0)) {
						if (valid < frame->frame[i]) {
							frame->frame[i] = valid;
						}

						valid -= frame->frame[i];
					}
				}

				frame->frames = i;
			} else if (size == base) {
				frame->frames = 0;
			}
		}
	} else {
		Free(*frameptr);
	}

	SET_POINTER(frameptr, frame);

	return frame;
}

void zmqframe_free(struct zmqframe *frame)
{
	if (unlikely(UNREFOBJ(frame))) {
		x_printf(W, "some data dose not be send.");
	}

	Free(frame);
}

int read_zmqdata(void *skt, struct zmqframe *frame)
{
	char            *buff = NULL;
	int             remain = 0;
	int             bytes = 0;
	volatile int    ret = 0;

	assert(skt && frame);

	if (unlikely(ISOBJ(frame))) {
		x_printf(W, "some data dose not be send.");
	}

	TRY
	{
		frame->frames = 0;
		frame->offset = offsetof(struct zmqframe, data);

		buff = frame->data;
		remain = frame->size - frame->offset;

		AssertError(remain > 0, ENOMEM);

		bytes = zmq_recv(skt, buff, remain, ZMQ_DONTWAIT);

		if (unlikely(bytes < 0)) {
			int code = errno;

			if (likely((code == EINTR) || (code == EAGAIN))) {
				ReturnValue(0);
			} else {
				RAISE_SYS_ERROR(bytes);
			}
		}

		AssertError(bytes <= remain && bytes <= PKG_FRAME_MAX, EMSGSIZE);

		/*保存第一帧的相关数据*/
		frame->frame[frame->frames++] = bytes;
		frame->offset += bytes;

		while (get_zmqopt(skt, ZMQ_RCVMORE)) {
			if (unlikely(frame->frames >= DIM(frame->frame))) {
				x_printf(W, "too much frame in package.");
				RAISE_SYS_ERROR_ERRNO(ENOMEM);
				break;
			}

			buff += bytes;
			remain -= bytes;
			AssertError(remain > 0, ENOMEM);

			bytes = zmq_recv(skt, buff, remain, 0);
			RAISE_SYS_ERROR(bytes);

			AssertError(bytes <= remain && bytes <= PKG_FRAME_MAX, EMSGSIZE);

			/*保存帧的相关数据*/
			frame->frame[frame->frames++] = bytes;
			frame->offset += bytes;
		}

		REFOBJ(frame);

		ret = frame->frames;
	}
	CATCH
	{
		ret = -1;
	}
	END;

	return ret;
}

int write_zmqdata(void *skt, struct zmqframe *frame)
{
	char            *buff = NULL;
	int             bytes = 0;
	int             offset = 0;
	volatile int    ret = -1;

	assert(skt && ISOBJ(frame) && frame->frame > 0);

	TRY
	{
		/*发送第一帧*/
		buff = frame->data;
		offset = offsetof(struct zmqframe, data);

		if (frame->frames == 1) {
			/*only one frame*/
			bytes = zmq_send(skt, buff, frame->frame[0], ZMQ_DONTWAIT);
		} else {
			bytes = zmq_send(skt, buff, frame->frame[0], ZMQ_DONTWAIT | ZMQ_SNDMORE);
		}

		if (unlikely(bytes < 0)) {
			int code = errno;

			if (likely((code == EINTR) || (code == EAGAIN))) {
				ReturnValue(0);
			} else {
				RAISE_SYS_ERROR(bytes);
			}
		}

		buff += frame->frame[0];
		offset += bytes;

		if (frame->frames == 1) {
			AssertError(offset == frame->offset, EINVAL);
			UNREFOBJ(frame);
			ReturnValue(1);
		}

		/*发送后续帧*/
		int i = 1;

		for (i = 1; i < frame->frames - 1; i++) {
			bytes = zmq_send(skt, buff, frame->frame[i], ZMQ_SNDMORE);
			RAISE_SYS_ERROR(bytes);
			buff += frame->frame[i];
			offset += bytes;
		}

		/*发送最后一帧*/
		bytes = zmq_send(skt, buff, frame->frame[i], 0);
		RAISE_SYS_ERROR(bytes);

		offset += bytes;
		AssertError(offset == frame->offset, EINVAL);
		ret = frame->frames;

		UNREFOBJ(frame);
	}
	CATCH
	{
		ret = -1;
	}
	END;

	return ret;
}

