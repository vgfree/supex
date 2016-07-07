/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "mfptp_parse.h"

#define		DECRYPTSIZE	1024		/* 解密缓冲区默认大小 ,如果数据大小大于此值，内部会自动加大缓冲区大小,此值取一个数据最常用的大小 */
#define		DECOMPRESSSIZE	1024		/* 解压缓冲区默认大小 ,如果数据大小大于此值，内部会自动加大缓冲区大小,此值取一个数据最常用的大小 */

static inline void _set_callback(struct mfptp_parser *parser);

void mfptp_parse_init(struct mfptp_parser *parser, char *const *data, const int *size)
{
	assert(parser && data && size);
	memset(parser, 0, sizeof(*parser));
	parser->ms.data = data;
	parser->ms.dsize = size;
	parser->ms.step = MFPTP_PARSE_INIT;
	commcache_init(&parser->ms.cache);
	parser->init = true;
	return ;
}

void mfptp_parse_destroy(struct mfptp_parser *parser)
{
	if (parser && parser->init) {
		commcache_free(&parser->ms.cache);
		parser->init = false;
	}
}

int mfptp_parse(struct mfptp_parser *parser)
{
	assert(parser && parser->init);

	int                             size_f_size = 0;		/* f_size字段所占字节数 */
	int                             frame_size = 0;			/* 解压解密之后的帧数据大小 */
	int				decomprsize = DECOMPRESSSIZE;	/* 解压缓冲区的大小 */
	int				decryptsize = DECRYPTSIZE;	/* 解密缓冲区的大小 */
	int                             dsize = *(parser->ms.dsize);	/* 待解析数据的大小 */
	const char                      *data = *parser->ms.data;	/* 待解析的数据缓冲区 */
	char				decryptbuff[DECRYPTSIZE] = {};		/* 保存解密之后的数据 */
	char				decompressbuff[DECOMPRESSSIZE] = {};	/* 保存解压之后的数据 */
	char*				decrypt = decryptbuff;		
	char*				decompress = decompressbuff;
	struct mfptp_package_info       *package = NULL;		/* 包的相关信息 */

	if (parser->ms.step == MFPTP_PARSE_OVER) {
		parser->ms.step = MFPTP_HEAD_FIRST;
	}

	if (parser->ms.step == MFPTP_HEAD_FIRST) {
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
			case MFPTP_PARSE_INIT:
			case MFPTP_HEAD_FIRST:
				if ((*parser->ms.data)[parser->ms.dosize] == '#') {
					parser->ms.step = MFPTP_HEAD_M;
				} else {
					log("bad protocol '#' wrong\n");
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				dsize -= 1;
				parser->ms.dosize += 1;
				break;

			case MFPTP_HEAD_M:
				if ((*parser->ms.data)[parser->ms.dosize] == 'M') {
					parser->ms.step = MFPTP_HEAD_F;
				} else {
					log("bad protocol 'M' wrong\n");
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				dsize -= 1;
				parser->ms.dosize += 1;
				break ;

			case MFPTP_HEAD_F:
				if ((*parser->ms.data)[parser->ms.dosize] == 'F') {
					parser->ms.step = MFPTP_HEAD_P;
				} else {
					log("bad protocol 'F' wrong\n");
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				dsize -= 1;
				parser->ms.dosize += 1;
				break ;

			case MFPTP_HEAD_P:
				if ((*parser->ms.data)[parser->ms.dosize] == 'P') {
					parser->ms.step = MFPTP_HEAD_T;
				} else {
					log("bad protocol 'P' wrong\n");
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				dsize -= 1;
				parser->ms.dosize += 1;
				break ;

			case MFPTP_HEAD_T:
				if ((*parser->ms.data)[parser->ms.dosize] == 'T') {
					parser->ms.step = MFPTP_HEAD_LAST;
				} else {
					log("bad protocol 'T' wrong\n");
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				dsize -= 1;
				parser->ms.dosize += 1;
				break ;

			case MFPTP_HEAD_LAST:
				if ((*parser->ms.data)[parser->ms.dosize] == 'P') {
					parser->ms.step = MFPTP_VERSION;
				} else {
					log("bad protocol last letter wrong\n");
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				dsize -= 1;
				parser->ms.dosize += 1;
				break ;

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
				parser->ms.step = MFPTP_F_SIZE_1;
				break;
#if 0
			case MFPTP_F_SIZE:
				if (dsize >= parser->header.size_f_size) {
					if (CHECK_F_SIZE(parser)) {
						parser->header.f_size = 0;
						for (size_f_size = parser->header.size_f_size-1; size_f_size > -1; size_f_size--) {
							parser->header.f_size = parser->header.f_size | ((unsigned char)data[parser->ms.dosize] << size_f_size * 8);
							parser->ms.dosize += 1;
						}

						dsize -= parser->header.size_f_size;
						if (parser->header.f_size < 1 || parser->header.f_size > MFPTP_MAX_DATASIZE) {
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
#endif
			case MFPTP_F_SIZE_1:
				parser->header.f_size = 0;
				if (CHECK_F_SIZE(parser)) {
					parser->header.f_size = (unsigned char)data[parser->ms.dosize];
					if (parser->header.size_f_size > 1) {
						parser->ms.step = MFPTP_F_SIZE_2;
					} else {
						parser->ms.step = MFPTP_FRAME_START;
					}
				} else {
					log("illegal datasize mfptp protocol frame carried\n");
					parser->ms.error = MFPTP_DATASIZE_INVAILD;
				}
				parser->ms.dosize += 1;
				dsize--;
				break ;

			case MFPTP_F_SIZE_2:
				parser->header.f_size = (parser->header.f_size << 8) | (unsigned char)data[parser->ms.dosize];
				if (parser->header.size_f_size > 2) {
					parser->ms.step = MFPTP_F_SIZE_3;
				} else {
					parser->ms.step = MFPTP_FRAME_START;
				}
				parser->ms.dosize += 1;
				dsize--;
				break ;

			case MFPTP_F_SIZE_3:
				parser->header.f_size = (parser->header.f_size << 8) | (unsigned char)data[parser->ms.dosize];
				if (parser->header.size_f_size > 3) {
					parser->ms.step = MFPTP_F_SIZE_4;
				} else {
					parser->ms.step = MFPTP_FRAME_START;
				}
				parser->ms.dosize += 1;
				dsize--;
				break ;

			case MFPTP_F_SIZE_4:
				parser->header.f_size = (parser->header.f_size << 8) | (unsigned char)data[parser->ms.dosize];
				parser->ms.step = MFPTP_FRAME_START;
				parser->ms.dosize += 1;
				dsize--;
				break ;

			case MFPTP_FRAME_START:
				if (CHECK_DATASIZE(parser)) {		/* 检测剩下的待解析数据是否大于等于f_size，检测返回false代表数据没有接收完毕 */
					if (parser->header.f_size > 1 && parser->header.f_size < MFPTP_MAX_FRAMESIZE) {
						/* 先解压 后解密 */
						if (parser->decompresscb) {
							parser->ms.step = MFPTP_FRAME_DECOMPRESS;
						} else if (parser->decryptcb){
							parser->ms.step = MFPTP_FRAME_DECRYPT;
						} else {
							parser->ms.step = MFPTP_FRAME_COPYDATA;
						}
						frame_size = parser->header.f_size;
					} else {
						/* 数据size大于帧允许的携带的数据大小 */
						parser->ms.error = MFPTP_DATASIZE_INVAILD;
						log("illegal datasize mfptp protocol frame carried\n");
					}
				} else {
					parser->ms.error = MFPTP_DATA_TOOFEW;
				}
				break ;

			case MFPTP_FRAME_DECOMPRESS:
				if (frame_size > DECOMPRESSSIZE) {
					decomprsize = frame_size;		/* 分配的空间大小待确定，实现了解压函数之后，根据相应算法重新赋值此变量 */
					NewArray(decompress, decomprsize);
					if (decompress == NULL) {
						parser->ms.error = MFPTP_DECOMPRESS_NO_SPACE;
						break ;
					}
				}
				frame_size = parser->decompresscb(decompress, &((*parser->ms.data)[parser->ms.dosize]), decomprsize, frame_size);
				if (parser->decryptcb) {
					parser->ms.step = MFPTP_FRAME_DECRYPT;
				} else {
					parser->ms.step = MFPTP_FRAME_COPYDATA;
				}
				break ;

			case MFPTP_FRAME_DECRYPT:
				if (frame_size > DECRYPTSIZE) {
					decryptsize = frame_size;		/* 分配的空间大小待确定，实现了解密函数之后，根据相应算法重新赋值此变量 */
					NewArray(decrypt, decryptsize);
					if (decrypt == NULL) {
						parser->ms.error = MFPTP_DECRYPT_NO_SPACE;
						break ;
					}
				}
				if (parser->decompresscb) {	
					/* 如果有解压，则源数据在解压缓冲区 */
					frame_size = parser->decryptcb(decrypt, decompress, decryptsize, frame_size);
				} else {
					frame_size = parser->decryptcb(decrypt, &((*parser->ms.data)[parser->ms.dosize]), decryptsize, frame_size);
				}
				parser->ms.step = MFPTP_FRAME_COPYDATA;
				break ;

			case MFPTP_FRAME_COPYDATA:
				if (parser->decryptcb == NULL && parser->decryptcb == NULL) {
					/* 未进行解压 解密 */
					commcache_append(&parser->ms.cache, &((*parser->ms.data)[parser->ms.dosize]), frame_size);
				} else if (parser->decompresscb && parser->decryptcb == NULL) {
					/* 只进行了解压 */
					commcache_append(&parser->ms.cache, decompress, frame_size);
				} else {
					commcache_append(&parser->ms.cache, decrypt, frame_size);
				}
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
				break ;

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
				parser->ms.step = MFPTP_HEAD_FIRST;
			}
			break;
		}
	}

	if (decrypt != decryptbuff) {
		Free(decrypt);
	}
	if (decompress != decompressbuff) {
		Free(decompress);
	}
	return parser->ms.dosize;
}

static int _decryptcb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	memcpy(destbuff, srcbuff, src_len);
	log("\x1B[1;32m""decrypt over\n""\x1B[m");
	return src_len;
}

static int _decompresscb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	memcpy(destbuff, srcbuff, src_len);
	log("\x1B[1;32m""decompress over\n""\x1B[m");
	return src_len;
}

static inline void _set_callback(struct mfptp_parser *parser)
{
	switch (parser->header.encryption)
	{
		case NO_ENCRYPTION:
			parser->decryptcb = NULL;
			break;

		case IDEA_ENCRYPTION:
			parser->decryptcb = _decryptcb;
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
			parser->decompresscb = _decompresscb;
			break;

		case GZIP_COMPRESSION:
			parser->decompresscb = NULL;
			break;

		default:
			parser->decompresscb = NULL;
			break;
	}
}
