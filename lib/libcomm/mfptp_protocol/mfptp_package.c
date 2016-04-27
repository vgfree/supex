/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "mfptp_package.h"

bool mfptp_packager_init(struct mfptp_packager_info *packager, char *buff, int *size)
{
	assert(packager && buff && size);
	memset(packager, 0, sizeof(*packager));
	packager->ms.buff = buff;
	packager->ms.size = size;
	packager->header.major_version = MFPTP_MAJOR_VERSION;
	packager->header.minor_version = MFPTP_MAINOR_VERSION;
	packager->init = true;
}

int mfptp_packager(struct mfptp_packager_info *packager, const char *data, unsigned char flag, int method)
{
	assert(packager && packager->init);
	packager->mp.header.packages = packager->mp.package.packages;
	packager->mp.compression = flag & (0x0F << 4);
	packager->mp.encryption = flag & 0x0F;
	packager->mp.header.socket_type = method;

	/* 赋值全部成功 开始打包 */
	int i = 0, j = 0, k = 0;
	int *index = packager->ms.size;	/* 用于保存待打包数据的缓冲区使用数据的大小 */
	for (i = 0; i < packager->mp.package.packages; i++) {
		/* 开始组包头 */
		memcpy(&packager->ms.buff[*packager->ms.size], "#MFPTP", 6);
		*packager->ms.size += 6;
		/* 版本号 */
		packager->buff[*packager->ms.size] = packager->header.major_version << 4 | packager->header.minor_version;
		*packager->ms.size += 1;
		/* 压缩加密设置 */
		packager->ms.buff[*packager->ms.size] = packager->mp.compression << 4 | packager->mp.encryption;
		*packager->ms.size += 1;

		/* socket工作类型 */
		packager->ms.buff[*packager->ms.size] = packager->ms.header.socket_type;
		*packager->ms.size += 1;

		/* socket包数 */
		packager->ms.buff[*packager->ms.size] = packager->ms.header.packages;
		*packager->ms.size += 1;

		/* 开始组装帧 */
		for(j = 0; j < packager->mp.package.frame[i].frames; j++) {
			int frame_size = packager->mp.package.frame[i].frame_size[j];
			int f_size = 0;	/* F_SIZE占几位 */
			/* FP_CONTROL的设置*/
			if (j == packager->mp.package.frame[i].frames-1) {
				/* 最后一帧 结束帧 */
				packager->ms.buff[*packager->ms.size] = 0x0;
			} else {
				packager->ms.buff[*packager->ms.size] = 0x1;
			}
			if (frame_size > 0 && frame_size < 256) {
				/* 一个字节*/
				f_size = 1;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x0;
			} else if (frame_size > 255 && frame_size < 65536 ) {
				/* 两个字节 */
				f_size = 2;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x1;
			} else if(frame_size > 65535 && frame_size < 16777216) {
				/* 三个字节 */
				f_size = 3;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x2;
			} else if (frame_size >16777215 && frame_size < 4294967296) {
				/* 四个字节 */
				f_size = 4;
				packager->ms.buff[*packager->ms.size] = packager->ms.buff[*packager->ms.size] << 2 | 0x3;
			} else {
				/* 出错 */
			}
			*packager->ms.size += 1;

			/*F_SIZE的设置 */
			for (k = 0; k < f_size; k++) {
				/**/
				packager->ms.buff[*packager->ms.size] = frame_size << (k*8);
				*packager->ms.size += 1;
			}
			memcpy(&packager->ms.buff[*packager->ms.size], &data[packager->mp.package.frame[i].frame_offset[j]], frame_size);
		}
		packager->ms.header.packages -= 1;
	}
}

bool mfptp_check_memory(int memsize, int packages, int frames, int dsize);
{
	int  size = 0;
	size = packages* MFPTP_HEADER_LEN + frames * (MFPTP_FP_CONTROL_LEN + MFPTP_F_SIZE_MAXLEN) + dsize;
	return size <= memsize ? true : false;
}


void mfptp_fill_package(struct mfptp_package_info *packager, const int *frame_offset, const int *frames_of_pack, int packages, int frames, int dsize)
{
	assert(packager && packager->init && frame_offset && frames_of_pack);
	int i, j;
	int k = 0;
	for(i = 0; i < packages; i++) {
		for(j = 0; j < frames_of_pack[i], j++, k++) {
			packager->mp.package.frame[i].frame_offset[j] = frame_offset[k];
			if (unlikely(k-1 == frames)) {
				package->mp.package.frame[i].frame_size[j] = dsize - frame_offset[k];

			} else {
				package->mp.package.frame[i].frame_size[j] = frame_offset[k+1] - frame_offset[k];
			}
			packager->mp.package.frame[i].frame_size[j] = frame_size[k];
		}
		packager->mp.package.frame[i].frames = frames_of_pack[i];
	}
	packager->mp.package.dsize = dsize;
	packager->mp.package.packages = packages;
	packager->mp.package.frames = frames;
}
