/*********************************************************************************************/
/************************	Created by 许莉 on 16/07/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_disposedata.h"

static int _encryptcb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	memcpy(destbuff, srcbuff, src_len);
	log("\x1B[1;32m""encrypt over\n""\x1B[m");
	return src_len;
}

static int _decryptcb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	memcpy(destbuff, srcbuff, src_len);
	log("\x1B[1;32m""decrypt over\n""\x1B[m");
	return src_len;
}

static int _compresscb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	int i = 0, k = 0; 
	int j = 0;
	for (i = 0; i < src_len; i++) {
		if (srcbuff[i] == ' ') {
			k++;
			continue ;
		}
		destbuff[j] = srcbuff[i];
		j++;
	}
	log("\x1B[1;32m""compress over\n""\x1B[m");
	return src_len-k;
}

static int _decompresscb(char *destbuff, const char *srcbuff, int dest_len, int src_len)
{
	int i = 0, j = 0, k = 0;
	for (i = 0; i < src_len; ) {
		if (j%3 == 0) {
			k++;
			destbuff[j] = ' ';
		} else {
			destbuff[j] = srcbuff[i];
			i++;
		}
		j++;
	}
	log("\x1B[1;32m""decompress over\n""\x1B[m");
	return src_len + k;
}

/* 设置加密压缩的回调函数 @flag:true 代表的是加密压缩， false代表的是解密解压 */
static inline DisposeData_CallBack  _set_callback(bool flag, unsigned char config)
{
	switch (config)
	{
		case NO_ENCRYPTION:		/* NO_ENCRYPTION 和 NO_COMPRESSION值都为0,所以只需要写一个 */
			return NULL;

		case IDEA_ENCRYPTION:
			if (flag) {
				return  NULL;	/* 待完善，指向IDEA算法加密函数 */
			} else {
				return  NULL;	/* 待完善，指向IDEA算法解密函数 */
			}

		case AES_ENCRYPTION:
			if (flag) {
				return  _encryptcb;	/* 待完善，指向AES算法加密函数 */
			} else {
				return  _decryptcb;	/* 待完善，指向AES算法的解密函数 */
			}

		case ZIP_COMPRESSION:
			if (flag) {
				return  _compresscb;		/* 待完善，指向ZIP算法压缩函数 */
			} else {
				return  _decompresscb;	/* 待完善，指向ZIP算法解压函数 */
			}

		case GZIP_COMPRESSION:
			if (flag) {
				return   NULL;	/* 待完善，指向GZIP算法压缩函数 */
			} else {
				return  NULL;	/* 待完善，指向GZIP算法解压函数 */
			}

		default:
			return NULL;
	}
}


/* 先加密 再压缩 */
bool  encrypt_compress_data(struct comm_cache *cache, struct comm_message *message)
{
	assert(cache && message);
	int	pckidx = 0;
	int	frmidx = 0;
	int	index = 0;
	int	frame_size = 0;
	int	encryptsize = ENCRYPTSIZE;	/* 加密缓冲区的大小 */
	int	compressize = COMPRESSIZE;	/* 压缩缓冲区的大小 */
	char	encryptbuff[ENCRYPTSIZE] = {};
	char	compressbuff[COMPRESSIZE] = {};
	char*	compress = compressbuff;
	char*	encrypt = encryptbuff;
	struct comm_message newmsg = {};
	DisposeData_CallBack	compresscb = NULL;
	DisposeData_CallBack	encryptcb = NULL;
	
	/* 先加密 再压缩 */
	if ((message->config & 0x0F) > 0) {
		/* 设置加密的回调函数 */
		encryptcb = _set_callback(true, message->config & 0xF0);
	}

	if ((message->config & 0xF0) > 0) {
		/* 设置压缩的回调函数 */
		compresscb = _set_callback(true, message->config & 0xF0);
	}

	if (compresscb == NULL && encryptcb == NULL) {
		return true;
	}
	for (pckidx = 0, index = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < message->package.frames_of_package[pckidx]; frmidx++,index++) {
			/* 对每帧的数据进行操作 */
			frame_size = message->package.frame_size[index];
			if (encryptcb) {
				if (frame_size > ENCRYPTSIZE) {
					encryptsize = frame_size;	/* 分配空间的大小待确定，实现了加密函数之后，根据相应的算法重复赋值此变量 */	
					if (encrypt != encryptbuff) {
						Free(encrypt);
					}
					NewArray(encrypt, encryptsize);
					if (encrypt == NULL) {
						log("no more space for encrypt data in mfptp package\n");
						return false;
					}
				}
				frame_size = encryptcb(encrypt, &message->content[message->package.frame_offset[index]], encryptsize, frame_size);
			}
			if (compresscb) {
				if (frame_size > DECOMPRESSIZE) {
					compressize = frame_size;	/* 分配空间的大小待确定，实现了压缩函数之后，根据相应的算法重复赋值此变量 */
					if (compress != compressbuff) {
						Free(compress);
					}
					NewArray(compress, compressize);
					if (compress == NULL) {
						log("no more space for compress data in mfptp package\n");
						return false;
					}
				}
				if (encryptcb) {
					frame_size = compresscb(compress, encrypt, compressize, frame_size);
				} else {
					frame_size = compresscb(compress, &message->content[message->package.frame_offset[index]], compressize, frame_size);
				}
			}
			newmsg.package.frame_offset[index] = newmsg.package.dsize;
			newmsg.package.frame_size[index] = frame_size;
			newmsg.package.dsize += frame_size;
			if (compresscb) {
				commcache_append(cache, compress, frame_size);
			} else {
				commcache_append(cache, encrypt, frame_size);
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

	if (newmsg.package.dsize != message->package.dsize) {
		message->content = realloc(message->content, newmsg.package.dsize);
		message->package.dsize = newmsg.package.dsize;
	}
	memcpy(message->content, cache->buffer, cache->size);
	memcpy(&message->package, &newmsg.package, sizeof(newmsg.package));
	cache->start += newmsg.package.dsize;
	cache->size -= newmsg.package.dsize;
	commcache_clean(cache);
	commcache_restore(cache);

	return true;
}





bool  decrypt_decompress_data(struct comm_cache *cache,  struct comm_message *message)
{
	assert(cache && message);
	int	pckidx = 0;
	int	frmidx = 0;
	int	index = 0;
	int	frame_size = 0;
	int	decomprsize = DECOMPRESSIZE;	/* 解压缓冲区的大小 */
	int	decryptsize = DECRYPTSIZE;	/* 解密缓冲区的大小 */
	char	decryptbuff[DECRYPTSIZE] = {};		/* 保存解密之后的数据 */
	char	decompressbuff[DECOMPRESSIZE] = {};	/* 保存解压之后的数据 */
	char*	decrypt = decryptbuff;		
	char*	decompress = decompressbuff;
	struct comm_message newmsg = {};
	DisposeData_CallBack	decompresscb = NULL;
	DisposeData_CallBack	decryptcb = NULL;
	
	/* 先解压 再解密 */
	if ((message->config & 0x0F) > 0) {
		/* 设置加密的回调函数 */
		decryptcb = _set_callback(false, message->config & 0xF0);
	}

	if ((message->config & 0xF0) > 0) {
		/* 设置压缩的回调函数 */
		decompresscb = _set_callback(false, message->config & 0xF0);
	}

	if (decompresscb == NULL && decryptcb == NULL) {
		return true;
	}

	for (pckidx = 0, index = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < message->package.frames_of_package[pckidx]; frmidx++, index++) {
			/* 对每帧的数据进行操作 */
			frame_size = message->package.frame_size[index];
			if (decompresscb) {
				if (frame_size > DECOMPRESSIZE) {
					decomprsize = frame_size;	/* 分配空间的大小待确定，实现了压缩函数之后，根据相应的算法重复赋值此变量 */
					if (decompress != decompressbuff) {
						Free(decompress);
					}
					NewArray(decompress, decomprsize);
					if (decompress == NULL) {
						log("no more space for decompress data in mfptp package\n");
						return false;
					}
				}
				frame_size = decompresscb(decompress, &message->content[message->package.frame_offset[index]], decomprsize, frame_size);
			}
			if (decryptcb) {
				if (frame_size > DECRYPTSIZE) {
					decryptsize = frame_size;	/* 分配空间的大小待确定，实现了加密函数之后，根据相应的算法重复赋值此变量 */	
					if (decrypt != decryptbuff) {
						Free(decrypt);
					}
					NewArray(decrypt, decryptsize);
					if (decrypt == NULL) {
						log("no more space for decrypt data in mfptp package\n");
						return false;
					}
				}
				if (decompresscb) {
					frame_size = decryptcb(decrypt, decompress, decryptsize, decomprsize);
				} else {
					frame_size = decryptcb(decrypt, &message->content[message->package.frame_offset[index]], decryptsize, frame_size);
				}
			}
			newmsg.package.frame_offset[index] = newmsg.package.dsize;
			newmsg.package.frame_size[index] = frame_size;
			newmsg.package.dsize += frame_size;
			if (decryptcb) {
				commcache_append(cache, decrypt, frame_size);
			} else {
				commcache_append(cache, decompress, frame_size);
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

	if (newmsg.package.dsize != message->package.dsize) {
		message->content = realloc(message->content, newmsg.package.dsize);
		message->package.dsize = newmsg.package.dsize;
	}
	memcpy(message->content, cache->buffer, cache->size);
	memcpy(&message->package, &newmsg.package, sizeof(newmsg.package));
	cache->start += newmsg.package.dsize;
	cache->size -= newmsg.package.dsize;
	commcache_clean(cache);
	commcache_restore(cache);

	return true;
}
