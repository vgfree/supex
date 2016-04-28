/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

bool mfptp_parse_init(struct mfptp_parser_info *parser, char* const *data, const int *size)
{
	assert(parser && *data && data && size);
	memset(parser, 0, sizeof(*parser));
	parser->ms.data = data;
	parser->ms.size = size;
	parser->ms.step = mfptp_init;
	parser->init = true;
	return true;
}

int mfptp_parse(struct mfptp_parser_info* parser)
{
	assert(parser);
	int	i = 0;
	const int dsize = *parser->ms.size;	/* 待解析数据的大小 */
	const unsigned char* data = *parser->ms.data;	/* 待解析的数据缓冲区 */
	while (dsize) {
		switch (parser->ms.step) {
			case mfptp_init:
			case mfptp_mfptp:
				if (likely(!strncmp(parser, &data[parser->ms.dosize], "#MFPTP", 6))) {
					parser->ms.step = mfptp_major_version;
				} else {
					parser->ms.error = mfptp_head_invaild;
				}
				break;
			case mfptp_version:
				parser->mp.header.major_version = data[parser->ms.dosize] & 0xF0;
				parser->mp.header.minor_version = data[parser->ms.dosize] & 0x0F;
				parser->ms.step = mfptp_config;
				break;
			case mfptp_config:
				parser->mp.header.compression = data[parser->ms.dosize] & 0xF0;
				parser->mp.header.encryption = data[parser->ms.dosize] & 0x0F;
				parser->ms.step = mfptp_socket_type;
				break ;
			case mfptp_socket_type:
				parser->mp.header.socket_type = data[parser->ms.dosize];
				parser->ms.step = mfptp_packages;
				break ;
			case mfptp_packages:
				parser->mp.header.packages = data[parser->ms.dosize];
				parser->ms.step = mfptp_fp_control;
				break ;
			case mfptp_fp_control:
				parser->mp.header.not_end = data[parser->ms.dosize] & 0x0C;
				parser->mp.header.size_f_size = data[parser->ms.dosize] & 0x03;
				parser->ms.step = mfptp_f_size;
				break;
			case mfptp_f_size:
				for(i = 0; i < parser->mp.header.size_f_size; i++) {
					parser->mp.header.f_size = parser->mp.header.f_size + data[parser->ms.dosize] << i*8 ;
				}
				break ;
			case mfptp_frame:
				parser->mp.package.frame[parser->mp.package.packages].frame_size[parser->mp.package.frame[parser->mp.package.packages].frames] = parser->mp.header.f_size;
				parser->package.frame[parser->mp.package.packages].frame_offset[parser->mp.package.frame[parser->mp.package.packages].frames]= parser->ms.dosize; 
				parser->package.frame[parser->mp.package.packages].frames ++;
				if (parser->mp.header.not_end) {
					parser->ms.step = mfptp_fp_control;

				} else {
					parser->ms.step = mfptp_frame_over;
				};
				break ;
			case mfptp_frame_over:
				parser->mp.package.packages ++;
				parser->ms.step = mfptp_parser;
				if (parser->mp.header.packages  == 0) {
					parser->ms.step = mfptp_package_over;
				}
				break ;
			case mfptp_package_over:
				paser->ms.error = PARSER_OK;
				break ;
		}
		dsize --;
		parser->ms.dosize++;
		if (parser->ms.error == PARSER_OK) {
			break ;
		}
	}

	return parser->ms.dosize;
}
