/*********************************************************************************************/
/************************	Created by 许莉 on 16/07/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "mfptp_protocol/mfptp_utils.h"

#include "comm_dispose_data.h"

#if 0
static int _encryptcb(char *dstbuff, const char *srcbuff, int dst_len, int src_len)
{
	memcpy(dstbuff, srcbuff, src_len);
	loger("\x1B[1;32m" "encrypt over\n" "\x1B[m");
	return src_len;
}

static int _decryptcb(char *dstbuff, const char *srcbuff, int dst_len, int src_len)
{
	memcpy(dstbuff, srcbuff, src_len);
	loger("\x1B[1;32m" "decrypt over\n" "\x1B[m");
	return src_len;
}

static int _compresscb(char *dstbuff, const char *srcbuff, int dst_len, int src_len)
{
	memcpy(dstbuff, srcbuff, src_len);
	loger("\x1B[1;32m" "compresscb over\n" "\x1B[m");
	return src_len;
}

static int _decompresscb(char *dstbuff, const char *srcbuff, int dst_len, int src_len)
{
	memcpy(dstbuff, srcbuff, src_len);
	loger("\x1B[1;32m" "decompresscb over\n" "\x1B[m");
	return src_len;
}
#endif /* if 0 */

/* 设置加密压缩的回调函数 @type:true 代表的是加密压缩， false代表的是解密解压 */
static inline DISPOSE_DATA_CALLBACK _set_callback(bool type, unsigned char flag)
{
	switch (flag) {
		case NO_ENCRYPTION:		/* NO_ENCRYPTION 和 NO_COMPRESSION值都为0,所以只需要写一个 */
			return NULL;

		case IDEA_ENCRYPTION:
			if (type) {
				return NULL;	/* 待完善，指向IDEA算法加密函数 */
			} else {
				return NULL;	/* 待完善，指向IDEA算法解密函数 */
			}

		case AES_ENCRYPTION:
			if (type) {
				//return _encryptcb;	/* 待完善，指向AES算法加密函数 */
				return NULL;	/* 待完善，指向AES算法加密函数 */
			} else {
				//return _decryptcb;	/* 待完善，指向AES算法的解密函数 */
				return NULL;	/* 待完善，指向AES算法的解密函数 */
			}

		case ZIP_COMPRESSION:
			if (type) {
				//return _compresscb;	/* 待完善，指向ZIP算法压缩函数 */
				return NULL;	/* 待完善，指向ZIP算法压缩函数 */
			} else {
				//return _decompresscb;	/* 待完善，指向ZIP算法解压函数 */
				return NULL;	/* 待完善，指向ZIP算法解压函数 */
			}

		case GZIP_COMPRESSION:
			if (type) {
				return NULL;	/* 待完善，指向GZIP算法压缩函数 */
			} else {
				return NULL;	/* 待完善，指向GZIP算法解压函数 */
			}

		default:
			return NULL;
	}
}

/* 先加密 再压缩 */
bool encrypt_compress_data(struct comm_message *message)
{
	assert(message);
	DISPOSE_DATA_CALLBACK    compresscb = NULL;
	DISPOSE_DATA_CALLBACK    encryptcb = NULL;

	if ((message->flags & 0x0F) > 0) {
		/* 设置加密的回调函数 */
		encryptcb = _set_callback(true, message->flags & 0x0F);
	}

	if ((message->flags & 0xF0) > 0) {
		/* 设置压缩的回调函数 */
		compresscb = _set_callback(true, message->flags & 0xF0);
	}

	/* 先加密 再压缩 */
	if ((compresscb == NULL) && (encryptcb == NULL)) {
		return true;
	}

	struct comm_message     newmsg = {0};
	commmsg_make(&newmsg, message->package.raw_data.len);
	newmsg.fd = message->fd;
	newmsg.flags = message->flags;
	newmsg.ptype = message->ptype;

	char                    encryptbuff[ENCRYPTSIZE] = {};
	char                    compressbuff[COMPRESSIZE] = {};
	char                    *compress = compressbuff;
	char                    *encrypt = encryptbuff;
	int                     encryptsize = ENCRYPTSIZE;      /* 加密缓冲区的大小 */
	int                     compressize = COMPRESSIZE;      /* 压缩缓冲区的大小 */

	int                     pckidx = 0;
	int                     frmidx = 0;
	int                     index = 0;
	int                     frame_size = 0;
	for (pckidx = 0, index = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < message->package.frames_of_package[pckidx]; frmidx++, index++) {
			/* 对每帧的数据进行操作 */
			frame_size = commmsg_frame_size(message, index);

			if (encryptcb) {
				if (frame_size > encryptsize) {
					encryptsize = frame_size;	/* 分配空间的大小待确定，实现了加密函数之后，根据相应的算法重复赋值此变量 */

					if (encrypt != encryptbuff) {
						Free(encrypt);
					}

					NewArray(encrypt, encryptsize);

					if (encrypt == NULL) {
						loger("no more space for encrypt data in mfptp package\n");
						return false;
					}
				}

				frame_size = encryptcb(encrypt, commmsg_frame_addr(message, index), encryptsize, frame_size);
			}

			if (compresscb) {
				if (frame_size > compressize) {
					compressize = frame_size;	/* 分配空间的大小待确定，实现了压缩函数之后，根据相应的算法重复赋值此变量 */

					if (compress != compressbuff) {
						Free(compress);
					}

					NewArray(compress, compressize);

					if (compress == NULL) {
						loger("no more space for compress data in mfptp package\n");
						return false;
					}
				}

				if (encryptcb) {
					frame_size = compresscb(compress, encrypt, compressize, frame_size);
				} else {
					frame_size = compresscb(compress, commmsg_frame_addr(message, index), compressize, frame_size);
				}
			}

			newmsg.package.frame_offset[index] = newmsg.package.raw_data.len;
			newmsg.package.frame_size[index] = frame_size;

			if (compresscb) {
				commsds_push_tail(&newmsg.package.raw_data, compress, frame_size);
			} else {
				commsds_push_tail(&newmsg.package.raw_data, encrypt, frame_size);
			}
		}

		newmsg.package.frames_of_package[pckidx] = message->package.frames_of_package[pckidx];
	}

	newmsg.package.packages = message->package.packages;
	newmsg.package.frames = message->package.frames;

	if (encrypt != encryptbuff) {
		Free(encrypt);
	}

	if (compress != compressbuff) {
		Free(compress);
	}

	commmsg_copy(message, &newmsg);
	commmsg_free(&newmsg);

	return true;
}

/* 先解压 再解密 */
bool decrypt_decompress_data(struct comm_message *message)
{
	assert(message);
	DISPOSE_DATA_CALLBACK    decompresscb = NULL;
	DISPOSE_DATA_CALLBACK    decryptcb = NULL;

	if ((message->flags & 0x0F) > 0) {
		/* 设置加密的回调函数 */
		decryptcb = _set_callback(false, message->flags & 0x0F);
	}

	if ((message->flags & 0xF0) > 0) {
		/* 设置压缩的回调函数 */
		decompresscb = _set_callback(false, message->flags & 0xF0);
	}

	/* 先解压 再解密 */
	if ((decompresscb == NULL) && (decryptcb == NULL)) {
		return true;
	}

	struct comm_message     newmsg = {0};
	commmsg_make(&newmsg, message->package.raw_data.len);
	newmsg.fd = message->fd;
	newmsg.flags = message->flags;
	newmsg.ptype = message->ptype;

	char                    decryptbuff[DECRYPTSIZE] = {};		/* 保存解密之后的数据 */
	char                    decompressbuff[DECOMPRESSIZE] = {};	/* 保存解压之后的数据 */
	char                    *decrypt = decryptbuff;
	char                    *decompress = decompressbuff;
	int                     decryptsize = DECRYPTSIZE;		/* 解密缓冲区的大小 */
	int                     decomprsize = DECOMPRESSIZE;		/* 解压缓冲区的大小 */

	int                     pckidx = 0;
	int                     frmidx = 0;
	int                     index = 0;
	int                     frame_size = 0;
	for (pckidx = 0, index = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < message->package.frames_of_package[pckidx]; frmidx++, index++) {
			/* 对每帧的数据进行操作 */
			frame_size = commmsg_frame_size(message, index);

			if (decompresscb) {
				if (frame_size > decomprsize) {
					decomprsize = frame_size;	/* 分配空间的大小待确定，实现了压缩函数之后，根据相应的算法重复赋值此变量 */

					if (decompress != decompressbuff) {
						Free(decompress);
					}

					NewArray(decompress, decomprsize);

					if (decompress == NULL) {
						loger("no more space for decompress data in mfptp package\n");
						return false;
					}
				}

				frame_size = decompresscb(decompress, commmsg_frame_addr(message, index), decomprsize, frame_size);
			}

			if (decryptcb) {
				if (frame_size > decryptsize) {
					decryptsize = frame_size;	/* 分配空间的大小待确定，实现了加密函数之后，根据相应的算法重复赋值此变量 */

					if (decrypt != decryptbuff) {
						Free(decrypt);
					}

					NewArray(decrypt, decryptsize);

					if (decrypt == NULL) {
						loger("no more space for decrypt data in mfptp package\n");
						return false;
					}
				}

				if (decompresscb) {
					frame_size = decryptcb(decrypt, decompress, decryptsize, frame_size);
				} else {
					frame_size = decryptcb(decrypt, commmsg_frame_addr(message, index), decryptsize, frame_size);
				}
			}

			newmsg.package.frame_offset[index] = newmsg.package.raw_data.len;
			newmsg.package.frame_size[index] = frame_size;

			if (decryptcb) {
				commsds_push_tail(&newmsg.package.raw_data, decrypt, frame_size);
			} else {
				commsds_push_tail(&newmsg.package.raw_data, decompress, frame_size);
			}
		}

		newmsg.package.frames_of_package[pckidx] = message->package.frames_of_package[pckidx];
	}

	newmsg.package.packages = message->package.packages;
	newmsg.package.frames = message->package.frames;

	if (decrypt != decryptbuff) {
		Free(decrypt);
	}

	if (decompress != decompressbuff) {
		Free(decompress);
	}

	commmsg_copy(message, &newmsg);
	commmsg_free(&newmsg);

	return true;
}

