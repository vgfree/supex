/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/28.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "mfptp_package.h"

#define	ENCRYPTSIZE	1024	/* 加密缓冲区的默认大小 */
#define	ENCOMPRESSSIZE	1024	/* 压缩缓冲区的默认大小 */

static inline void _set_callback(struct mfptp_packager *packager);

static inline bool _check_config(struct mfptp_packager *packager);

static void _make_package(struct mfptp_packager *packager, struct mfptp_package_info *package, const char *data);

void mfptp_package_init(struct mfptp_packager *packager, char **buff, int *size)
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

void mfptp_package_destroy(struct mfptp_packager *packager)
{
	if (packager && packager->init) {
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
	int pckidx = 0;

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
	int size = 0;

	size = MFPTP_HEADER_LEN + frames * (MFPTP_FP_CONTROL_LEN + MFPTP_F_SIZE_MAXLEN) + dsize;
	return size - memsize;
}

bool mfptp_fill_package(struct mfptp_packager *packager, const int *frame_offset, const int *frame_size, const int *frames_of_pack, int packages, int datasize)
{
	assert(packager && packager->init && frame_offset && frame_size && frames_of_pack);
	int     index = 0;
	int     dsize = 0;	/* 数据的总大小 */
	int     pckidx = 0;	/* 包的索引 */
	int     frmidx = 0;	/* 帧的索引 */

	for (pckidx = 0; pckidx < packages; pckidx++) {
		if (frames_of_pack[pckidx] > 0) {
			for (frmidx = 0 ; frmidx < frames_of_pack[pckidx]; frmidx++, index++) {
				if (frame_size[index] <= datasize && dsize < datasize) {
					if (frame_offset[index] < datasize) {
						packager->bodyer.package[pckidx].frame[frmidx].frame_offset = frame_offset[index];
						packager->bodyer.package[pckidx].frame[frmidx].frame_size = frame_size[index];
						dsize += frame_size[index];
					} else {

						log("wrong frame_offset in comm_package\n");
						return false;
					}
				} else {
					log("wrong frame_size in comm_package frame_size:0x%x\n", frame_size[index]);
					return false;
				}
			}
			packager->bodyer.package[pckidx].frames = frames_of_pack[pckidx];
		} else {
			log("wrong sum of package in comm_packages\n");
			return false;
		}
	}
	if (dsize == datasize) {
		packager->bodyer.dsize = dsize;
		packager->bodyer.packages = packages;
		return true;
	} else {
		log("wrong sum of datasize in comm_package\n");
		return false;
	}
}

/* 将帧组合成一个包数据 @package:此包数据的相关信息 @data：需要进行打包的数据 */
static void _make_package(struct mfptp_packager *packager, struct mfptp_package_info *package, const char *data)
{
	int     frmidx = 0;				/* 帧的索引 */
	int     frame_size = 0;				/* 帧的大小 */
	int     size_f_size = 0;			/* f_size字段占几位 */
	int	encryptsize = 0;			/* 加密缓冲区的大小 */
	int	compressize = 0;			/* 压缩缓冲区的大小 */
	char	encryptbuff[ENCRYPTSIZE] = {};		/* 加密缓冲区 */
	char	compressbuff[ENCOMPRESSSIZE] = {};	/* 压缩缓冲区 */
	char*	encrypt = encryptbuff;
	char*	compress = compressbuff;

	/* 开始组装帧 */
	for (frmidx = 0; frmidx < package->frames; frmidx++) {
		if (package->frame[frmidx].frame_size < 1 || package->frame[frmidx].frame_size > MFPTP_MAX_FRAMESIZE) {
			/* 数据size大于允许帧所携带的数据大小或小于零 */
			packager->ms.error = MFPTP_DATASIZE_INVAILD;
			log("illegal datasize mfptp protocol frame carried\n");
			return;
		}
		frame_size = package->frame[frmidx].frame_size;
		/* 先加密 再压缩 */
		if (packager->encryptcb) {
			if (frame_size > ENCRYPTSIZE) {
				encryptsize = frame_size;	/* 分配空间的大小待确定，实现了加密函数之后，根据相应的算法重复赋值此变量 */	
				NewArray(encrypt, encryptsize);
				if (encrypt == NULL) {
					packager->ms.error = MFPTP_ENCRYPT_NO_SPACE;
					log("no more space for encrypt data in mfptp package\n");
					return ;
				}
			}
			frame_size = packager->encryptcb(encrypt, &data[package->frame[frmidx].frame_offset], encryptsize, frame_size);
		}
		if (packager->compresscb) {
			if (frame_size > ENCOMPRESSSIZE) {
				compressize = frame_size;	/* 分配空间的大小待确定，实现了压缩函数之后，根据相应的算法重复赋值此变量 */
				NewArray(compress, compressize);
				if (compress == NULL) {
					packager->ms.error = MFPTP_ENCOMPRESS_NO_SPACE;
					log("no more space for compress data in mfptp package\n");
					return ;
				}
			}
			if (packager->encryptcb) {
				frame_size = packager->compresscb(compress, encrypt, compressize, frame_size);
			} else {
				frame_size = packager->compresscb(compress, &data[package->frame[frmidx].frame_offset], compressize, frame_size);
			}
		}

		/* FP_CONTROL的设置 */
		if (frmidx == package->frames - 1) {
			/* 最后一帧 结束帧 */
			(*packager->ms.buff)[*packager->ms.size] = 0x0;
		} else {
			(*packager->ms.buff)[*packager->ms.size] = 0x1;
		}

		/* 确定f_size字段占几个字节 */
		if ((frame_size > 0) && (frame_size < 256)) {
			/* 一个字节 */
			packager->header.size_f_size = 1;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x0;
		} else if ((frame_size > 255) && (frame_size < 65536)) {
			/* 两个字节 */
			packager->header.size_f_size = 2;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x1;
		} else if ((frame_size > 65535) && (frame_size < 16777216)) {
			/* 三个字节 */
			packager->header.size_f_size = 3;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x2;
		} else if ((frame_size > 16777215) && (frame_size < 4294967296)) {
			/* 四个字节 */
			packager->header.size_f_size = 4;
			(*packager->ms.buff)[*packager->ms.size] = (*packager->ms.buff)[*packager->ms.size] << 2 | 0x3;
		} else {
			packager->ms.error = MFPTP_DATASIZE_INVAILD;
			log("illegal datasize mfptp protocol frame carried\n");
			break;
		}

		*packager->ms.size += 1;
		packager->ms.dosize += 1;

		/*F_SIZE字段的设置 */
		for(size_f_size = packager->header.size_f_size-1; size_f_size > -1; size_f_size--) {
			(*packager->ms.buff)[*packager->ms.size] = (frame_size >> (size_f_size * 8)) & 0xFF;
			*packager->ms.size += 1;
		}

		packager->ms.dosize += size_f_size;

		
		if (packager->encryptcb == NULL && packager->compresscb == NULL) {
			memcpy(&((*packager->ms.buff)[*packager->ms.size]), &data[package->frame[frmidx].frame_offset], frame_size);
		} else if (packager->encryptcb && packager->compresscb == NULL){
			memcpy(&((*packager->ms.buff)[*packager->ms.size]), encrypt, frame_size);
		} else {
			memcpy(&((*packager->ms.buff)[*packager->ms.size]), compress, frame_size);
		}

		*packager->ms.size += frame_size;
		packager->ms.dosize += frame_size;

		if (encrypt != encryptbuff) {
			Free(encrypt);
		}
		if (compress != compressbuff) {
			Free(compress);
		}
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
		packager->ms.error = MFPTP_SOCKTYPE_INVAILD;
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

static int _encryptcb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	memcpy(destbuff, srcbuff, src_len);
	log("\x1B[1;32m""decrypt over\n""\x1B[m");
	return src_len;
}

static int _compresscb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	memcpy(destbuff, srcbuff, src_len);
	log("\x1B[1;32m""decompress over\n""\x1B[m");
	return src_len;
}
/* 设置加密压缩的回调函数 */
static inline void _set_callback(struct mfptp_packager *packager)
{
	switch (packager->header.encryption)
	{
		case NO_ENCRYPTION:
			packager->encryptcb = NULL;
			break;

		case IDEA_ENCRYPTION:
			packager->encryptcb = _encryptcb;
			break;

		case AES_ENCRYPTION:
			packager->encryptcb = NULL;
			break;

		default:
			packager->encryptcb = NULL;
			break;
	}

	switch (packager->header.compression)
	{
		case NO_COMPRESSION:
			packager->compresscb = NULL;
			break;

		case ZIP_COMPRESSION:
			packager->compresscb = _compresscb;
			break;

		case GZIP_COMPRESSION:
			packager->compresscb = NULL;
			break;

		default:
			packager->compresscb = NULL;
			break;
	}
}

