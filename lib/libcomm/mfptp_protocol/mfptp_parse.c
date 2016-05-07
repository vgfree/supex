/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "mfptp_parse.h"

static inline void _set_callback(struct mfptp_parser *parser);

void mfptp_parse_init(struct mfptp_parser *parser, char* const *data, const int *size)
{
	assert(parser && *data && data && size);
	memset(parser, 0, sizeof(*parser));
	parser->ms.data = data;
	parser->ms.dsize = size;
	parser->ms.step = MFPTP_PARSE_INIT;
	parser->init = true;
	return ;
}

int mfptp_parse(struct mfptp_parser *parser)
{
	assert(parser && parser->init);

	int		i = 0;
	int		frame_size = 0;				/* 解压解密之后的帧数据大小 */
	int		frame_offset = parser->ms.cache.size;	/* 每帧在cache里面的偏移 */
	int		dsize = *(parser->ms.dsize);		/* 待解析数据的大小 */
	const char*	data = *(parser->ms.data);		/* 待解析的数据缓冲区 */
	struct mfptp_frame_info* frame = NULL;			/* 帧的相关信息 */
	char decryptbuff[MFPTP_MAX_FRAMESIZE] = {0};		/* 用于保存解密之后的数据 */
	char decompressbuff[MFPTP_MAX_FRAMESIZE] = {0};		/* 用于保存解压之后的数据 */

	if (parser->ms.error == MFPTP_PARSER_OVER) {
		/* 外部需要检测error的状态，为MFPTP_PARSER_OVER时就必须清理一下cache */
		parser->ms.dosize = 0;
	}
	while (dsize) {
		switch (parser->ms.step) {
			case MFPTP_HEAD:
			case MFPTP_PARSE_INIT:
				if (likely(CHECK_HEADER(parser))) {
					dsize -= 6;
					parser->ms.dosize += 6;
					parser->ms.step = MFPTP_VERSION;
				} else {
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				break;
			case MFPTP_VERSION:
				parser->header.major_version = data[parser->ms.dosize] & 0xF0;
				parser->header.minor_version = data[parser->ms.dosize] & 0x0F;
				if (likely(CHECK_VERSION(parser))) {
					dsize --;
					parser->ms.dosize += 1;
					parser->ms.step = MFPTP_CONFIG;
				} else {
					parser->ms.error = MFPTP_VERSION_INVAILD;
				}
				break;
			case MFPTP_CONFIG:
				parser->header.compression = data[parser->ms.dosize] & 0xF0;
				parser->header.encryption = data[parser->ms.dosize] & 0x0F;
				if (likely(CHECK_CONFIG(parser))) {
					dsize --;
					parser->ms.dosize += 1;
					_set_callback(parser);
					parser->ms.step = MFPTP_SOCKET_TYPE;
				} else {
					parser->ms.error = MFPTP_CONFIG_INVAILD;
				}
				break ;
			case MFPTP_SOCKET_TYPE:
				parser->header.socket_type = data[parser->ms.dosize];
				if (likely(CHECK_SOCKTYPE(parser))) {
					dsize --;
					parser->ms.dosize += 1;
					parser->ms.step = MFPTP_PACKAGES;
				} else {
					parser->ms.error = MFPTP_SOCKTYPE_INVAILD;
				}
				break ;
			case MFPTP_PACKAGES:
				parser->header.packages = data[parser->ms.dosize];
				if (likely(CHECK_PACKAGES(parser))) {
					dsize --;
					parser->ms.dosize += 1;
					parser->ms.step = MFPTP_FP_CONTROL;
				} else {
					parser->ms.error = MFPTP_PACKAGES_TOOMUCH;
				}
				break ;
			case MFPTP_FP_CONTROL:
				parser->header.not_end = data[parser->ms.dosize] & 0x0C;
				parser->header.size_f_size = data[parser->ms.dosize] & 0x03;
				dsize --;
				parser->ms.dosize += 1;
				parser->ms.step = MFPTP_F_SIZE;
				break;
			case MFPTP_F_SIZE:
				for(i = 0; i < parser->header.size_f_size; i++) {
					parser->header.f_size = parser->header.f_size + (data[parser->ms.dosize] << i*8) ;
				}
				parser->ms.dosize += parser->header.size_f_size;
				dsize -= parser->header.size_f_size;
				parser->ms.step = MFPTP_FRAME_START;
				break ;
			case MFPTP_FRAME_START:
				/* 先解压 后解密 */
				if (likely(CHECK_FRAMESIZE(parser))) { /* 检测帧的数据是否等于f_size，不等于代表数据没有接收完毕 */
					if (parser->decompresscb) {
						frame_size = parser->decompresscb(decompressbuff, parser->ms.data[parser->ms.dosize], MFPTP_MAX_FRAMESIZE, parser->header.f_size);
					} else {
						memcpy(decompressbuff, parser->ms.data[parser->ms.dosize], parser->header.f_size);
						frame_size = parser->header.f_size;
					}
					if (parser->decryptcb) {
						frame_size = parser->decryptcb(decryptbuff, decompressbuff, MFPTP_MAX_FRAMESIZE, frame_size);
					} else {
						memcpy(decryptbuff, decompressbuff, frame_size);
					}

					commcache_append(&parser->ms.cache, decryptbuff, frame_size);

					frame = &(parser->package.frame[parser->package.packages]);
					frame->frame_size[frame->frames] = frame_size;
					frame->frame_offset[frame->frames] = frame_offset;

					frame->frames++;
					frame_offset += frame_size;
					parser->ms.dosize += parser->header.f_size;
					dsize -= parser->header.f_size;

					if (parser->header.not_end) {
						parser->ms.step = MFPTP_FP_CONTROL;
					} else {
						parser->ms.step = MFPTP_FRAME_OVER;
					}
				} else {
					parser->ms.error = MFPTP_DATA_TOOFEW;
				}
				break ;
			case MFPTP_FRAME_OVER:
				parser->package.packages ++;
				if (parser->header.packages  == 0) {
					parser->ms.step = MFPTP_PACKAGE_OVER;
				} else {
					parser->ms.step = MFPTP_PARSE_INIT;
				}
				break ;
			case MFPTP_PACKAGE_OVER:
				parser->ms.step = MFPTP_PARSE_OVER;
				break ;
		}

		if (parser->ms.error != MFPTP_OK ) {
			/* 解析过程中出错，直接退出 */
			break ;
		} else if (parser->ms.step == MFPTP_PARSE_OVER) {
			/* 代表此次所有连续的两个包都已经解析完毕 即使后面还有其他的数据 都不进行解析了 */
			parser->ms.step = MFPTP_PARSE_INIT;
			break ;
		}
	}
	return parser->ms.dosize;
}

static inline void _set_callback(struct mfptp_parser *parser) 
{
	switch (parser->header.encryption) {
		case NO_ENCRYPTION:
			parser->decryptcb = NULL;
			break ;
		case IDEA_ENCRYPTION:
			break ;
		case AES_ENCRYPTION:
			break ;
		default:
			parser->decryptcb = NULL;
			break ;
	}

	switch (parser->header.compression) {
		case NO_COMPRESSION:
			parser->decompresscb = NULL;
			break ;
		case ZIP_COMPRESSION:
			break ;
		case GZIP_COMPRESSION:
			break ;
		default:
			parser->decompresscb = NULL;
			break ;
	}
}
