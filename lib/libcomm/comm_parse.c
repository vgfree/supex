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

	/* 有数据进行解析的时候才去进行解析并将数据全部解析完毕 */
	while (commdata->recv_cache.size > 0) {
		flag = false;
		size = mfptp_parse(&commdata->parser);
		if (likely(size > 0 && commdata->parser.ms.error == MFPTP_OK && commdata->parser.ms.step == MFPTP_PARSE_OVER)) {	/* 成功解析了一个连续的包 */
			message = new_commmsg(commdata->parser.package.dsize);
			if (likely(message)) {
				message->fd = commdata->commtcp.fd;
				memcpy(message->content, &commdata->parser.ms.cache.buffer[commdata->parser.ms.cache.start], commdata->parser.package.dsize);
				_fill_message_package(message, &commdata->parser);
				commdata->parser.ms.cache.start += commdata->parser.package.dsize;
				commdata->parser.ms.cache.size -= commdata->parser.package.dsize;
				commdata->recv_cache.start += size;
				commdata->recv_cache.size -= size;
				commcache_clean(&commdata->recv_cache);
				commcache_clean(&commdata->parser.ms.cache);
				commlock_lock(&commctx->recvlock);
				if (likely(commqueue_push(&commctx->recv_queue, (void*)&message))) {
					if (unlikely(!commctx->recv_queue.readable)) {			/* 为0代表有线程在等待可读 */
						/* 唤醒在commctx->recv_queue.readable上等待的线程并设置其为1 */
						commlock_wake(&commctx->recvlock, &commctx->recv_queue.readable, 1, true);
					}
					flag = true;
				} else {
					/* 解析数据的时候队列已满，另作处理 不进行堵塞等待用户取数据 */
					//flag = false;
					flag = true;
				}
				commlock_unlock(&commctx->recvlock);
				if (!flag) {
					break ;
				}
				log("parse successed\n");
			} 
		} else if (commdata->parser.ms.error != MFPTP_DATA_TOOFEW) {
			/* 解析出错 抛弃已解析的错误数据  */
			flag = false;
			commdata->recv_cache.start += size;
			commdata->recv_cache.size -= size;
			commcache_clean(&commdata->recv_cache);
			log("parse failed\n");
			commdata->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
			break ;
		} else if (commdata->parser.ms.error == MFPTP_DATA_TOOFEW) {
			commdata->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
			break ;
		}
	}
#if 0
	/* 解析成功了 */
	if (flag ) {
		commdata->parser.ms.dosize = 0;
		memset(&commdata->parser.package, 0, sizeof(commdata->parser.package));
		commcache_clean(&commdata->recv_cache);
		commcache_clean(&commdata->parser.ms.cache);
	}
#endif
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
			message->package.frame_offset[k] = package->frame[pckidx].frame_offset[frmidx] - parser->ms.cache.start;	/* 每段偏移都是从此缓冲区的*/
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
