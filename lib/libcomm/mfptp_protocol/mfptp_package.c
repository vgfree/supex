/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "mfptp_package.h"

static inline void _set_callback(struct mfptp_packager *packager);

static inline bool _check_config(struct mfptp_packager *packager);

static void _make_package(struct mfptp_packager *packager, struct mfptp_package_info *package, const char *data);

bool  mfptp_package_init(struct mfptp_packager *packager, char **buff, int *size)
{
	assert(packager && buff && size);
	memset(packager, 0, sizeof(*packager));
	packager->ms.buff = buff;
	packager->ms.size = size;
	NewArray(packager->encryptbuff, MFPTP_MAX_DATASIZE);
	if (unlikely(!packager->encryptbuff)) {
		return false;
	}
	NewArray(packager->compressbuff, MFPTP_MAX_DATASIZE);
	if (unlikely(!packager->compressbuff)) {
		Free(packager->encryptbuff);
		return false;
	}
	packager->header.major_version = MFPTP_MAJOR_VERSION;
	packager->header.minor_version = MFPTP_MINOR_VERSION;
	packager->init = true;
	return true;
}

void mfptp_package_destroy(struct mfptp_packager *packager) 
{
	if (packager && packager->init) {
		Free(packager->encryptbuff);
		Free(packager->compressbuff);
		packager->init = false;
	}
}

int mfptp_package(struct mfptp_packager *packager, const char *data, unsigned char flag, int method)
{
	assert(packager && packager->init && data);

	packager->header.packages = packager->bodyer.packages;
	packager->header.compression = flag & 0xF0;
	packager->header.encryption = flag & 0x0F;
	packager->header.socket_type = method;
	packager->ms.dosize = 0;

	/* 检测一下所有的配置是否合法 */
	if (unlikely(!_check_config(packager))) {
		return -1;
	}

	/* 设置加密压缩的回调函数 */
	_set_callback(packager);

	/* 开始组装header */
	memcpy(&((*packager->ms.buff)[*packager->ms.size]), "#MFPTP", 6);
	*packager->ms.size += 6;
	packager->ms.dosize += 6;
	/* 版本号 */
	(*packager->ms.buff)[*packager->ms.size] = packager->header.major_version << 4 | packager->header.minor_version;
	*packager->ms.size += 1;
	packager->ms.dosize += 1;
	/* 压缩加密设置 */
	(*packager->ms.buff)[*packager->ms.size] = packager->header.compression | packager->header.encryption;
	*packager->ms.size += 1;
	packager->ms.dosize += 1;

	/* socket工作类型 */
	(*packager->ms.buff)[*packager->ms.size] = packager->header.socket_type;
	*packager->ms.size += 1;
	packager->ms.dosize += 1;

	/* 设置包数 */
	(*packager->ms.buff)[*packager->ms.size] = packager->header.packages;
	*packager->ms.size += 1;
	packager->ms.dosize += 1;

	/* 开始组装bodyer */
	int  pckidx = 0;
	for (pckidx = 0; pckidx < packager->bodyer.packages; pckidx++) {
		/* 依次组装每包数据 */
		_make_package(packager, &packager->bodyer.package[pckidx], data);
		if (packager->ms.error != MFPTP_OK) {
			return -1;
		}
	}
	return packager->ms.dosize;
}

int mfptp_check_memory(int memsize, int frames, int dsize)
{
	int  size = 0;
	size = MFPTP_HEADER_LEN + frames * (MFPTP_FP_CONTROL_LEN + MFPTP_F_SIZE_MAXLEN) + dsize;
	return size - memsize;
}

void mfptp_fill_package(struct mfptp_packager *packager, const int *frame_offset, const int *frame_size, const int *frames_of_package, int packages) 
{
	assert(packager && packager->init && frame_offset && frame_size);
	int index	= 0;
	int dsize	= 0;	/* 数据的总大小 */
	int pckidx	= 0;	/* 包的索引 */
	int frmidx	= 0;	/* 帧的索引 */
	for (pckidx = 0; pckidx < packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, index++) {
			packager->bodyer.package[pckidx].frame[frmidx].frame_offset = frame_offset[index];
			packager->bodyer.package[pckidx].frame[frmidx].frame_size = frame_size[index];
			dsize += frame_size[index];
		}
		packager->bodyer.package[pckidx].frames = frames_of_package[pckidx];
	}
	packager->bodyer.dsize = dsize;
	packager->bodyer.packages = packages;
}

/* 将帧组合成一个包数据 @package:此包数据的相关信息 @data：需要进行打包的数据 */
static void _make_package(struct mfptp_packager *packager, struct mfptp_package_info *package, const char *data)
{
	int frmidx	= 0;				/* 帧的索引 */
	int frame_size	= 0;				/* 帧的大小 */
	int size_f_size	= 0;				/* f_size字段占几位 */

	/* 开始组装帧 */
	for(frmidx = 0; frmidx < package->frames; frmidx++) {
		/* 先加密 再压缩 */
		if (package->frame[frmidx].frame_size > MFPTP_MAX_DATASIZE) {
			/* 数据size大于允许帧所携带的数据大小 */
			packager->ms.error = MFPTP_DATASIZE_INVAILD;
			log("illegal datasize mfptp protocol frame carried\n");
			return ;
		}
		if (packager->encryptcb) {
			frame_size = packager->encryptcb(packager->encryptbuff, &data[package->frame[frmidx].frame_offset], MFPTP_MAX_DATASIZE, package->frame[frmidx].frame_size);
		} else {
			memcpy(packager->encryptbuff, &data[package->frame[frmidx].frame_offset], package->frame[frmidx].frame_size);
			frame_size = package->frame[frmidx].frame_size;
		}
		
		if (packager->compresscb) {
			frame_size = packager->compresscb(packager->compressbuff, packager->encryptbuff, MFPTP_MAX_DATASIZE, frame_size);
		} else {
			memcpy(packager->compressbuff, packager->encryptbuff, frame_size);
		}

		/* FP_CONTROL的设置 */
		if (frmidx == package->frames-1) {
			/* 最后一帧 结束帧 */
			(*packager->ms.buff)[*packager->ms.size] = 0x0;
		} else {
			(*packager->ms.buff)[*packager->ms.size] = 0x1;
		}

		/* 确定f_size字段占几个字节 */
		if (frame_size > 0 && frame_size < 256) {
			/* 一个字节 */
			packager->header.size_f_size = 1;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x0;
		} else if (frame_size > 255 && frame_size < 65536 ) {
			/* 两个字节 */
			packager->header.size_f_size = 2;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x1;
		} else if(frame_size > 65535 && frame_size < 16777216) {
			/* 三个字节 */
			packager->header.size_f_size = 3;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x2;
		} else if (frame_size >16777215 && frame_size < 4294967296) {
			/* 四个字节 */
			packager->header.size_f_size = 4;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x3;
		} else {
			packager->ms.error = MFPTP_DATASIZE_INVAILD;
			log("illegal datasize mfptp protocol frame carried\n");
			break ;
		}
		*packager->ms.size += 1;
		packager->ms.dosize += 1;

		/*F_SIZE字段的设置 */
		for (size_f_size = 0; size_f_size < packager->header.size_f_size; size_f_size++) {
			(*packager->ms.buff)[*packager->ms.size] = (frame_size >> (size_f_size *8)) & 0xFF;
			*packager->ms.size += 1;
		}
		packager->ms.dosize += size_f_size;

		memcpy(&((*packager->ms.buff)[*packager->ms.size]), packager->compressbuff, frame_size);
		*packager->ms.size += frame_size;
		packager->ms.dosize += frame_size;
	}
}

/* 检测MFPTP所有的配置是否合法 */
static inline bool _check_config(struct mfptp_packager *packager) 
{
	if (unlikely(!CHECK_VERSION(packager))) {
		packager->ms.error = MFPTP_VERSION_INVAILD;
		log("bad mfptp protocol version\n");
		return false;
	}

	if (unlikely(!CHECK_CONFIG(packager))) {
		log("bad mfptp protocol compression or encryption setting\n");
		packager->ms.error = MFPTP_CONFIG_INVAILD;
		return false;
	}


	if (unlikely((!CHECK_SOCKTYPE(packager)))) {
		packager->ms.error= MFPTP_SOCKTYPE_INVAILD;
		log("bad mfptp protocol socket type\n");
		return false;
	}

	if (unlikely(!CHECK_PACKAGES(packager))) {
		log("illegal mfptp protocol packages\n");
		packager->ms.error = MFPTP_PACKAGES_INVAILD;
		return false;
	}
	return true;
}

/* 设置加密压缩的回调函数 */
static inline void _set_callback(struct mfptp_packager *packager)
{
	switch (packager->header.encryption) {
		case NO_ENCRYPTION:
			packager->encryptcb = NULL;
			break ;
		case IDEA_ENCRYPTION:
			packager->encryptcb = NULL;
			break ;
		case AES_ENCRYPTION:
			packager->encryptcb = NULL;
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
			packager->compresscb = NULL;
			break ;
		case GZIP_COMPRESSION:
			packager->compresscb = NULL;
			break ;
		default:
			packager->compresscb = NULL;
			break ;
	}
}
