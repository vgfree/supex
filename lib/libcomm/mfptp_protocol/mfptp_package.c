/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "mfptp_package.h"

static inline void _set_callback(struct mfptp_packager *packager);

void  mfptp_package_init(struct mfptp_packager *packager, char *buff, int *size)
{
	assert(packager && buff && size);
	memset(packager, 0, sizeof(*packager));
	packager->ms.buff = buff;
	packager->ms.size = size;
	packager->header.major_version = MFPTP_MAJOR_VERSION;
	packager->header.minor_version = MFPTP_MINOR_VERSION;
	packager->init = true;
	return ;
}

int mfptp_package(struct mfptp_packager *packager, const char *data, unsigned char flag, int method)
{
	assert(packager && packager->init && data);
	int	i = 0, j = 0, k = 0;
	int	frame_size			= 0;	/* 加密压缩之后帧数据的大小  */
	int	size_f_size			= 0;	/* F_SIZE占几位 */
	struct mfptp_frame_info frame		= {0};	/* 帧的相关信息 */
	char encryptbuff[MFPTP_MAX_FRAMESIZE]	= {0};	/* 用于保存加密之后的数据 */
	char compressbuff[MFPTP_MAX_FRAMESIZE]	= {0};	/* 用于保存压缩之后的数据 */

	packager->header.packages = packager->package.packages;
	packager->header.compression = flag & 0xF0;
	packager->header.encryption = flag & 0x0F;
	packager->header.socket_type = method;

	_set_callback(packager);
	if (CHECK_VERSION(packager)) {
	} else {
		packager->ms.error = MFPTP_VERSION_INVAILD;
		return -1;
	}
	if (CHECK_CONFIG(packager)) {
	} else {
		packager->ms.error = MFPTP_CONFIG_INVAILD;
		return -1;
	}

	if (CHECK_SOCKTYPE(packager)) {
	} else {
		packager->ms.error= MFPTP_CONFIG_INVAILD;
		return -1;
	}

	if (CHECK_PACKAGES(packager)) {
	} else {
		packager->ms.error = MFPTP_PACKAGES_TOOMUCH;
		return -1;
	}

	for (i = 0; i < packager->package.packages; i++) {
		/* 开始组包头 */
		memcpy(&packager->ms.buff[*packager->ms.size], "#MFPTP", 6);
		*packager->ms.size += 6;
		/* 版本号 */
		packager->ms.buff[*packager->ms.size] = packager->header.major_version << 4 | packager->header.minor_version;
		*packager->ms.size += 1;
		/* 压缩加密设置 */
		packager->ms.buff[*packager->ms.size] = packager->header.compression << 4 | packager->header.encryption;
		*packager->ms.size += 1;

		/* socket工作类型 */
		packager->ms.buff[*packager->ms.size] = packager->header.socket_type;
		*packager->ms.size += 1;

		/* socket包数 */
		packager->ms.buff[*packager->ms.size] = packager->header.packages;
		*packager->ms.size += 1;

		/* 开始组装帧 */
		for(j = 0; j < packager->package.frame[i].frames; j++) {
			frame = packager->package.frame[i];
			/* 先加密 再压缩 */
			if (packager->encryptcb) {
				memset(encryptbuff, 0,  MFPTP_MAX_FRAMESIZE);
				frame_size = packager->encryptcb(encryptbuff, &data[frame.frame_offset[j]], MFPTP_MAX_FRAMESIZE, frame.frame_size[j]);	
			} else {
				memcpy(encryptbuff, &data[frame.frame_offset[j]], frame.frame_size[j]);
				frame_size = frame.frame_size[j];
			}
			
			if (packager->compresscb) {
				memset(compressbuff, 0, MFPTP_MAX_FRAMESIZE);
				frame_size = packager->compresscb(compressbuff, encryptbuff, MFPTP_MAX_FRAMESIZE, frame_size);
			} else {
				memcpy(compressbuff, encryptbuff, frame_size);
			}
			/* FP_CONTROL的设置 */
			if (j == packager->package.frame[i].frames-1) {
				/* 最后一帧 结束帧 */
				packager->ms.buff[*packager->ms.size] = 0x0;
			} else {
				packager->ms.buff[*packager->ms.size] = 0x1;
			}
			if (frame_size > 0 && frame_size < 256) {
				/* 一个字节 */
				size_f_size = 1;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x0;
			} else if (frame_size > 255 && frame_size < 65536 ) {
				/* 两个字节 */
				size_f_size = 2;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x1;
			} else if(frame_size > 65535 && frame_size < 16777216) {
				/* 三个字节 */
				size_f_size = 3;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x2;
			} else if (frame_size >16777215 && frame_size < 4294967296) {
				/* 四个字节 */
				size_f_size = 4;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x3;
			} else {
				packager->ms.error = MFPTP_DATA_TOOMUCH;
				break ;
			}
			*packager->ms.size += 1;

			/*F_SIZE的设置 */
			for (k = 0; k < size_f_size; k++) {
				/**/
				packager->ms.buff[*packager->ms.size] = frame_size << (k*8);
				*packager->ms.size += 1;
			}
			memcpy(&packager->ms.buff[*packager->ms.size], compressbuff, frame_size);
			*packager->ms.size += frame_size;
		}
		if (packager->ms.error != MFPTP_OK) {
			packager->package.dsize = -1;
			break ;
		}
		packager->header.packages -= 1;
	}
	return packager->package.dsize;
}

int mfptp_check_memory(int memsize, int packages, int frames, int dsize)
{
	int  size = 0;
	size = packages* MFPTP_HEADER_LEN + frames * (MFPTP_FP_CONTROL_LEN + MFPTP_F_SIZE_MAXLEN) + dsize;
	return memsize-size;
}

void mfptp_fill_package(struct mfptp_packager *packager, const int *frame_offset, const int *frame_size, const int *frames_of_package, int packages) 
{
	assert(packager && packager->init && frame_offset && frame_size);
	int dsize = 0;
	int i = 0, j = 0, k = 0;
	for (i = 0; i < packages; i++) {
		for (j = 0; j < frames_of_package[i]; j++) {
			packager->package.frame[i].frame_offset[j] = frame_offset[k];
			packager->package.frame[i].frame_size[j] = frame_size[k];
			dsize += frame_size[k];
			k++;
		}
		packager->package.frame[i].frames = frames_of_package[i];
	}
	packager->package.dsize = dsize;
	packager->package.packages = packages;
}

static inline void _set_callback(struct mfptp_packager *packager)
{
	switch (packager->header.encryption) {
		case NO_ENCRYPTION:
			packager->encryptcb = NULL;
			break ;
		case IDEA_ENCRYPTION:
			break ;
		case AES_ENCRYPTION:
			break ;
		default:
			packager->encryptcb = NULL;
			break ;
	}

	switch (packager->header.compression) {
		case NO_COMPRESSION:
			packager->compresscb = NULL;
			break ;
		case ZIP_COMPRESSION:
			break ;
		case GZIP_COMPRESSION:
			break ;
		default:
			packager->compresscb = NULL;
			break ;
	}
}
