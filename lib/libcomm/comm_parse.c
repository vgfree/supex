/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_parse.h"

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *packager);

bool parse_data(struct comm_data *commdata)
{
	assert(commdata);
	bool			flag = false;
	bool			block = false;
	int			size = 0;	/* 成功解析数据的字节数 */
	struct comm_context	*commctx = commdata->commctx;
	struct comm_message	*message = NULL;

	/* 有数据进行解析的时候才去进行解析 */
	while (commdata->recv_buff.size > 0) {
		size = mfptp_parse(&commdata->parser);
		if (likely(size > 0 && commdata->parser.ms.error == MFPTP_OK)) {	/* 解析成功 */
			message = new_commmsg(commdata->parser.package.dsize);
			if (likely(message)) {
				message->fd = commdata->portinfo.fd;
				memcpy(message->content, &commdata->parser.ms.cache.cache[commdata->parser.ms.cache.start], commdata->parser.package.dsize);
				_fill_message_package(message, &commdata->parser);
				commdata->parser.ms.cache.start += commdata->parser.package.dsize;
				commdata->parser.ms.cache.size -= commdata->parser.package.dsize;
				commdata->recv_buff.start += size;
				commdata->recv_buff.size -= size;
				commcache_clean(&commdata->recv_buff);
				commcache_clean(&commdata->parser.ms.cache);
				commlock_lock(&commctx->recvlock);
				if (likely(commqueue_push(&commctx->recv_queue, (void*)&message))) {
					if (unlikely(!commctx->recv_queue.readable)) {			/* 为0代表有线程在等待可读 */
						commlock_wake(&commctx->recvlock, &commctx->recv_queue.readable, 1, true);
					}
					flag = true;
				} else {
					/* 解析数据的时候队列已满，另作处理 不进行堵塞等待用户取数据 */
#if 0
					commctx->recv_queue.writeable = 0;
					if (likely(commlock_wait(&commctx->recvlock, &commctx->recv_queue.writeable, 1, timeout, true))) {
						block = true;
						continue ;
					} else {
						break ;
					}
					if (unlikely(block && commctx->recv_queue.nodes == commctx->recv_queue.capacity)) {
						commctx->recv_queue.writeable = 0;
					}
#endif
					flag = false;
				}
				commlock_unlock(&commctx->recvlock);
				log("parse successed\n");
				if (!flag) {
					break ;
				}
			} 
		} else if (commdata->parser.ms.error != MFPTP_DATA_TOOFEW) {
			/* 解析出错 抛弃已解析的错误数据  */
			flag = false;
			commdata->recv_buff.start += size;
			commdata->recv_buff.size -= size;
			commcache_clean(&commdata->recv_buff);
			log("parse failed\n");
			break ;
		}
	}
	return flag;
}

/* 填充message结构体 */ 
static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser) 
{
	assert(message && parser);
	int k = 0;
	int pckidx = 0;		/* 包的索引 */
	int frmidx = 0;		/* 帧的缩影 */
	int frames = 0;		/* 总帧数 */
	const struct mfptp_package_info* package = &parser->package;
	const struct mfptp_header_info* header = &parser->header;
	for (pckidx = 0; pckidx < package->packages; pckidx++) {
		for (frmidx = 0; frmidx < package->frame[pckidx].frames; frmidx++, k++) {
			message->package.frame_size[k] = package->frame[pckidx].frame_size[frmidx];
			message->package.frame_offset[k] = package->frame[pckidx].frame_offset[frmidx];
		}
		message->package.frames_of_package[pckidx] = package->frame[pckidx].frames;
		frames += package->frame[pckidx].frames;
	}
	message->package.packages = package->packages;
	message->package.frames = frames;
	message->package.dsize = package->dsize;

	message->config = header->compression | header->encryption;
	message->socket_type = header->socket_type;
}
