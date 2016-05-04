/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "mfptp_parse.h"

bool mfptp_parse_init(struct mfptp_parser_info *parser, char* const *data, const int *size)
{
	assert(parser && *data && data && size);
	memset(parser, 0, sizeof(*parser));
	parser->ms.data = data;
	parser->ms.dsize = size;
	parser->ms.step = MFPTP_PARSE_INIT;
	parser->init = true;
	return true;
}

int mfptp_parse(struct mfptp_parser_info* parser)
{
	assert(parser && parser->init);
	int	i = 0;
	int dsize = *(parser->ms.dsize);	/* 待解析数据的大小 */
	const unsigned char* data = *(parser->ms.data);	/* 待解析的数据缓冲区 */
	while (dsize) {
		switch (parser->ms.step) {
			case MFPTP_PARSE_INIT:
			case MFPTP_HEAD:
				if (likely(!strncmp(&data[parser->ms.dosize], "#MFPTP", 6))) {
					parser->ms.step = MFPTP_VERSION;
				} else {
					parser->ms.error = MFPTP_HEAD_INVAILD;
				}
				break;
			case MFPTP_VERSION:
				parser->mp.header.major_version = data[parser->ms.dosize] & 0xF0;
				parser->mp.header.minor_version = data[parser->ms.dosize] & 0x0F;
				parser->ms.step = MFPTP_CONFIG;
				break;
			case MFPTP_CONFIG:
				parser->mp.header.compression = data[parser->ms.dosize] & 0xF0;
				parser->mp.header.encryption = data[parser->ms.dosize] & 0x0F;
				parser->ms.step = MFPTP_SOCKET_TYPE;
				break ;
			case MFPTP_SOCKET_TYPE:
				parser->mp.header.socket_type = data[parser->ms.dosize];
				parser->ms.step = MFPTP_PACKAGES;
				break ;
			case MFPTP_PACKAGES:
				parser->mp.header.packages = data[parser->ms.dosize];
				parser->ms.step = MFPTP_FP_CONTROL;
				break ;
			case MFPTP_FP_CONTROL:
				parser->mp.header.not_end = data[parser->ms.dosize] & 0x0C;
				parser->mp.header.size_f_size = data[parser->ms.dosize] & 0x03;
				parser->ms.step = MFPTP_F_SIZE;
				break;
			case MFPTP_F_SIZE:
				for(i = 0; i < parser->mp.header.size_f_size; i++) {
					parser->mp.header.f_size = parser->mp.header.f_size + (data[parser->ms.dosize] << i*8) ;
				}
				break ;
			case MFPTP_FRAME:
				parser->mp.package.frame[parser->mp.package.packages].frame_size[parser->mp.package.frame[parser->mp.package.packages].frames] = parser->mp.header.f_size;
				parser->mp.package.frame[parser->mp.package.packages].frame_offset[parser->mp.package.frame[parser->mp.package.packages].frames]= parser->ms.dosize; 
				parser->mp.package.frame[parser->mp.package.packages].frames ++;
				if (parser->mp.header.not_end) {
					parser->ms.step = MFPTP_FP_CONTROL;

				} else {
					parser->ms.step = MFPTP_FRAME_OVER;
				};
				break ;
			case MFPTP_FRAME_OVER:
				parser->mp.package.packages ++;
				parser->ms.step = MFPTP_PARSE_INIT;
				if (parser->mp.header.packages  == 0) {
					parser->ms.step = MFPTP_PACKAGE_OVER;
				}
				break ;
			case MFPTP_PACKAGE_OVER:
				parser->ms.error = MFPTP_PARSER_OVER;
				break ;
		}
		dsize --;
		parser->ms.dosize++;
		if (parser->ms.error != MFPTP_PARSER_OK || parser->ms.error == MFPTP_PARSER_OVER) {
			parser->ms.step = MFPTP_PARSE_INIT;
			break ;
		}
	}

	return parser->ms.dosize;
}
