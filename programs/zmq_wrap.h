//
//  zmq_wrap.h
//  supex
//
//  Created by 周凯 on 15/12/9.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef zmq_wrap_h
#define zmq_wrap_h

#include "libmini.h"

__BEGIN_DECLS
/*数据每次增长大小*/
#define PKG_INCREMENT   (512)
/*数据包帧最大数量*/
#define PKG_MAX_FRAMES  (32)
/*单帧最大数据量*/
#define PKG_FRAME_MAX   (UINT16_MAX)
struct zmqframe
{
	int             magic;			/*数据是否有效*/
	int             size;			/*分配数据总长度（包括头）*/
	int             offset;			/*当前有效数据长度（包括头）*/
	int             frames;			/*帧数*/
	uint16_t        frame[PKG_MAX_FRAMES];	/*每帧大小*/
	char            data[];			/*接收／发送数据起始位置*/
};

int get_zmqopt(void *skt, int opt);

void set_zmqopt(void *skt, int opt, int value);

bool check_zmqwrite(void *skt, int to);

bool check_zmqread(void *skt, int to);

struct zmqframe *zmqframe_new(int size);

struct zmqframe *zmqframe_resize(struct zmqframe **frame, int addsize);

void zmqframe_free(struct zmqframe *frame);

/**
 * 返回接收的帧数 > 0 or = 0(again) or -1(error)
 * 如果单帧数据太大或帧的数据过多，则设置EMSGSIZE／ENOMEM系统错误
 */
int read_zmqdata(void *skt, struct zmqframe *iocache);

/**
 * 返回发送的帧数 > 0 or = 0(again) or -1(error)
 * 如果单帧数据太大或帧的数据过多，则设置EMSGSIZE／ENOMEM系统错误
 */
int write_zmqdata(void *skt, struct zmqframe *iocache);

__END_DECLS
#endif	/* zmq_wrap_h */

