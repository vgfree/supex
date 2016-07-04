/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "mfptp_parse.h"

static inline void _set_callback(struct mfptp_parser *parser);

bool mfptp_parse_init(struct mfptp_parser *parser, char *const *data, const int *size)
{
	assert(parser && data && size);
	memset(parser, 0, sizeof(*parser));
	parser->ms.data = data;
	parser->ms.dsize = size;
	parser->ms.step = MFPTP_PARSE_INIT;
	commcache_init(&parser->ms.cache);
	NewArray(parser->decryptbuff, MFPTP_MAX_DATASIZE);

	if (unlikely(!parser->decryptbuff)) {
		return false;
	}

	NewArray(parser->decompressbuff, MFPTP_MAX_DATASIZE);

	if (unlikely(!parser->decompressbuff)) {
		Free(parser->decryptbuff);
		return false;
	}

	parser->init = true;
	return true;
}

void mfptp_parse_destroy(struct mfptp_parser *parser)
{
	if (parser && parser->init) {
		commcache_free(&parser->ms.cache);
		Free(parser->decryptbuff);
		Free(parser->decompressbuff);
		parser->init = false;
	}
}

int mfptp_parse(struct mfptp_parser *parser)
{
	assert(parser && parser->init);

	int                             size_f_size = 0;		/* f_size字段所占字节数 */
	int                             frame_size = 0;			/* 解压解密之后的帧数据大小 */
	int                             dsize = *(parser->ms.dsize);	/* 待解析数据的大小 */
	const char                      *data = *parser->ms.data;	/* 待解析的数据缓冲区 */
	struct mfptp_package_info       *package = NULL;		/* 包的相关信息 */

	if (parser->ms.step == MFPTP_PARSE_OVER) {
		parser->ms.step = MFPTP_HEAD;
	}

	if (parser->ms.step == MFPTP_HEAD) {
		/* 从头开始解析的时候就要恢复一下变量的初始化值 */
		parser->ms.error = MFPTP_OK;
		parser->ms.dosize = 0;
		parser->ms.frame_offset = 0;
		/* 只要解析完成一次就需要立即去提取解析完的数据，然后将bodyer和cache清零 */
		parser->ms.cache.start += parser->bodyer.dsize;
		parser->ms.cache.size -= parser->bodyer.dsize;
		commcache_clean(&parser->ms.cache);
		memset(&parser->bodyer, 0, sizeof(parser->bodyer));
	}

	while (dsize) {
		switch ((int)parser->ms.step)
		{
			case MFPTP_HEAD:
			case MFPTP_PARSE_INIT:

				if (dsize >= 6) {
					if (CHECK_HEADER(parser)) {
						parser->ms.step = MFPTP_VERSION;
						dsize -= 6;
						parser->ms.dosize += 6;
					} else {
						log("bad protocol\n");
						int index = 0;
						for (index = 0; index < 6; index++) {
							if ((*parser->ms.data)[parser->ms.dosize] == '#') {
								if (index == 0) {
									dsize -= 1;
									parser->ms.dosize += 1;
								}
								break ;
							}
							dsize -= 1;
							parser->ms.dosize += 1;
						}
						parser->ms.error = MFPTP_HEAD_INVAILD;
					}
				} else {
					parser->ms.error = MFPTP_DATA_TOOFEW;	/* 数据没有接收完毕 */
				}

				break;

			case MFPTP_VERSION:
				parser->header.major_version = data[parser->ms.dosize] >> 4;
				parser->header.minor_version = data[parser->ms.dosize] & 0x0F;

				if (CHECK_VERSION(parser)) {
					parser->ms.step = MFPTP_CONFIG;
				} else {
					log("bad mfptp protocol version\n");
					parser->ms.error = MFPTP_VERSION_INVAILD;
				}
				dsize--;
				parser->ms.dosize += 1;

				break;

			case MFPTP_CONFIG:
				parser->header.compression = data[parser->ms.dosize] & 0xF0;
				parser->header.encryption = data[parser->ms.dosize] & 0x0F;

				if (CHECK_CONFIG(parser)) {
					_set_callback(parser);
					parser->ms.step = MFPTP_SOCKET_TYPE;
				} else {
					log("bad mfptp protocol compression or encryption setting\n");
					parser->ms.error = MFPTP_CONFIG_INVAILD;
				}
				dsize--;
				parser->ms.dosize += 1;

				break;

			case MFPTP_SOCKET_TYPE:
				parser->header.socket_type = data[parser->ms.dosize];

				if (CHECK_SOCKTYPE(parser)) {
					if (parser->header.socket_type == HEARTBEAT_METHOD) {
						parser->ms.step = MFPTP_PARSE_OVER;
					} else {
						parser->ms.step = MFPTP_PACKAGES;
					}
				} else {
					log("bad mfptp protocol socket type\n");
					parser->ms.error = MFPTP_SOCKTYPE_INVAILD;
				}
				dsize--;
				parser->ms.dosize += 1;

				break;

			case MFPTP_PACKAGES:
				parser->header.packages = data[parser->ms.dosize];

				if (CHECK_PACKAGES(parser)) {
					parser->ms.step = MFPTP_FP_CONTROL;
				} else {
					log("illegal mfptp protocol packages\n");
					parser->ms.error = MFPTP_PACKAGES_INVAILD;
				}
				dsize--;
				parser->ms.dosize += 1;

				break;

			case MFPTP_FP_CONTROL:
				parser->header.not_end = (data[parser->ms.dosize] & 0x0C) >> 2;
				parser->header.size_f_size = (data[parser->ms.dosize] & 0x03) + 0x01;
				dsize--;
				parser->ms.dosize += 1;
				parser->ms.step = MFPTP_F_SIZE;
				break;

			case MFPTP_F_SIZE:

				if (dsize >= parser->header.size_f_size) {
					if (CHECK_F_SIZE(parser)) {
						parser->header.f_size = 0;
						for (size_f_size = 0; size_f_size < parser->header.size_f_size; size_f_size++) {
							parser->header.f_size = parser->header.f_size | ((unsigned char)data[parser->ms.dosize] << size_f_size * 8);
							parser->ms.dosize += 1;
						}

						dsize -= parser->header.size_f_size;
						if (parser->header.f_size > MFPTP_MAX_DATASIZE) {
							/* 数据size大于帧允许的携带的数据大小 */
							parser->ms.error = MFPTP_DATASIZE_INVAILD;
							log("illegal datasize mfptp protocol frame carried\n");
						} else {
							parser->ms.step = MFPTP_FRAME_START;
						}
					} else {
						parser->ms.dosize += parser->header.size_f_size;
						log("illegal datasize mfptp protocol frame carried\n");
						parser->ms.error = MFPTP_DATASIZE_INVAILD;
					}
				} else {
					parser->ms.error = MFPTP_DATA_TOOFEW;
				}

				break;

			case MFPTP_FRAME_START:

				if (CHECK_DATASIZE(parser)) {		/* 检测剩下的待解析数据是否大于等于f_size，检测返回false代表数据没有接收完毕 */
					/* 先解压 后解密 */
					if (parser->decompresscb) {
						frame_size = parser->decompresscb(parser->decompressbuff, &((*parser->ms.data)[parser->ms.dosize]), MFPTP_MAX_DATASIZE, parser->header.f_size);
					} else {
						memcpy(parser->decompressbuff, &((*parser->ms.data)[parser->ms.dosize]), parser->header.f_size);
						frame_size = parser->header.f_size;
					}

					if (parser->decryptcb) {
						frame_size = parser->decryptcb(parser->decryptbuff, parser->decompressbuff, MFPTP_MAX_DATASIZE, frame_size);
					} else {
						memcpy(parser->decryptbuff, parser->decompressbuff, frame_size);
					}

					commcache_append(&parser->ms.cache, parser->decryptbuff, frame_size);
					package = &parser->bodyer.package[parser->bodyer.packages];
					package->frame[package->frames].frame_size = frame_size;
					package->frame[package->frames].frame_offset = parser->ms.frame_offset;
					package->frames++;

					parser->ms.frame_offset += frame_size;
					parser->bodyer.dsize += frame_size;
					parser->ms.dosize += parser->header.f_size;

					if (parser->header.not_end) {
						dsize -= parser->header.f_size;
						parser->ms.step = MFPTP_FP_CONTROL;
					} else {
						parser->ms.step = MFPTP_FRAME_OVER;
					}
				} else {
					parser->ms.error = MFPTP_DATA_TOOFEW;
				}

				break;

			case MFPTP_FRAME_OVER:
				dsize -= parser->header.f_size;
				parser->bodyer.packages++;

				if (parser->header.packages == parser->bodyer.packages) {	/* 代表所有的包都已经解析完毕 */
					parser->ms.step = MFPTP_PARSE_OVER;
				} else {
					parser->ms.step = MFPTP_FP_CONTROL;
				}

				break;
		}

		if (parser->ms.step == MFPTP_PARSE_OVER) {
			/* 已解析完毕 直接退出 */
			break;
		} else if ((dsize == 0) && (parser->ms.error == MFPTP_OK)) {
			/* 没有解析完毕 但待解析数据字节数已为0 */
			parser->ms.error = MFPTP_DATA_TOOFEW;
			break;
		} else if (parser->ms.error != MFPTP_OK) {
			/* 解析出错 直接退出 */
			if (parser->ms.error != MFPTP_DATA_TOOFEW) {
				/* 如果错误不属于数据不足 其他错误则直接将数据清零 并从头开始解析 */
				parser->ms.step = MFPTP_HEAD;
			}

			break;
		}
	}

	return parser->ms.dosize;
}

static inline void _set_callback(struct mfptp_parser *parser)
{
	switch (parser->header.encryption)
	{
		case NO_ENCRYPTION:
			parser->decryptcb = NULL;
			break;

		case IDEA_ENCRYPTION:
			parser->decryptcb = NULL;
			break;

		case AES_ENCRYPTION:
			parser->decryptcb = NULL;
			break;

		default:
			parser->decryptcb = NULL;
			break;
	}

	switch (parser->header.compression)
	{
		case NO_COMPRESSION:
			parser->decompresscb = NULL;
			break;

		case ZIP_COMPRESSION:
			parser->decompresscb = NULL;
			break;

		case GZIP_COMPRESSION:
			parser->decompresscb = NULL;
			break;

		default:
			parser->decompresscb = NULL;
			break;
	}
}

