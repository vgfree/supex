#include "mfptp_pack.h"

/* 名  称: mfptp_init_pack_info;
 * 功  能: 初始化结构体变量;
 * 参  数: p_info指针指向struct mfptp_pack结构体;
 * 返回值: 无
 * 作  者: chenliuliu    at 2015/10/15
 */

#define PTR_MOVE(len)		      \
	do {			      \
		offset += len;	      \
		p_info->doptr += len; \
	} while (0)

int mfptp_pack_len(int len)
{
	return len + 128;
}

void mfptp_init_pack_info(struct mfptp_pack *p_info)
{
	p_info->compress = 0;
	p_info->encrypt = 0;
	p_info->method = 0;
	p_info->packages = 0;
	p_info->sub_version = 0;
	p_info->version = 0;
	p_info->step = 0;
	p_info->doptr = 0;
}

/* 名  称: mfptp_pack_set_packages
 * 功  能: 设置要打包的包的个数
 * 参  数: count, 包的个数;
 *         p_info指针指向struct mfptp_pack结构体，内含控制变量packages;
 * 返回值: 无
 * 作  者: chenliuliu    at 2015/10/15
 */
void mfptp_pack_set_packages(int count, struct mfptp_pack *p_info)
{
	p_info->packages = count;
}

/* 名  称: mfptp_pack_frames_with_packages
 * 功  能: 将接收到的数据打包成mfptp协议格式,并放到数组中
 * 参  数: src, 指向要打包的数据; len, 要打包数据的长度;
 *         dst,添加包头后的数据写入到该地址;
 *         more,非零表示有后续帧,零表示无后续帧;
 *         p_info指针指向struct mfptp_pack结构体,内含控制变量
 * 返回值: MFPTP_PACK_PACKAGE  代表还有后续帧或包;
 *         MFPTP_PACK_OVER     打包结束;
 *         MFPTP_PACK_ERROR    打包出错;
 * 作  者: chenliuliu    at 2015/10/15
 */
int mfptp_pack_frames_with_packages(char *src, int len, char *dst, int more, struct mfptp_pack *p_info)
{
	assert(src);
	assert(dst);
	assert(p_info);
	short   ver;
	uint8_t encry;
	int     de_len;
	int     out_len;
	char    *buf = NULL;
	char    *buf1 = NULL;
#define MFPTP_FRAME_MAX_LEN (1024 * 1024 * 6)

	if (1) {
		buf1 = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == buf1) {
			printf("加密内存分配错误\n");
			return MFPTP_PACK_ERROR;
		}
	}

	if (1) {
		buf = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == buf) {
			printf("压缩内存分配错误\n");
			free(buf1);
			return MFPTP_PACK_ERROR;
		}
	}

	int             offset = p_info->doptr;	/* 指向打包的数据字段数量 */
	unsigned char   *ptr = dst;

	while (1) {
		switch (p_info->step)
		{
			case    MFPTP_PACK_INIT:
			case    MFPTP_PACK_HEAD:
			{
				/* 计算加密压缩控制位  */
				uint8_t p_compress = p_info->compress << 4;
				p_compress |= 0x0f;
				encry = p_info->encrypt | 0xf0;
				p_compress &= encry;

				/* 计算版本控制位 */
				uint8_t p_version = ((uint8_t)p_info->version) << 4;
				p_version |= 0x0f;
				ver = p_info->sub_version | 0xf0;
				p_version &= (uint8_t)ver;

				/* 包头前六位必须是#MFPTP */
				memcpy(ptr, "#MFPTP", 6);
				PTR_MOVE(6);

				/* 版本  */
				*(ptr + offset) = p_version;
				PTR_MOVE(1);

				/* 加密压缩  */
				*(ptr + offset) = p_compress;
				PTR_MOVE(1);

				/* socket 方法  */
				*(ptr + offset) = p_info->method;
				PTR_MOVE(1);

				/* 包的个数  */
				*(ptr + offset) = p_info->packages;
				PTR_MOVE(1);
				p_info->step = MFPTP_PACK_PACKAGE;
				break;
			}

			case    MFPTP_PACK_PACKAGE:
			{
				if (p_info->compress == 0x02) {
					de_len = mfptp_gzip(buf, MFPTP_FRAME_MAX_LEN, src, len);
				} else if (p_info->compress == 0x01) {
					de_len = mfptp_zip(buf, MFPTP_FRAME_MAX_LEN, src, len);
				} else {
					de_len = len;
					memcpy(buf, src, len);
				}

				if (p_info->encrypt == 0x02) {
					out_len = mfptp_aes_ecb_encrypt_evp(p_info->m_key.key, 16, buf, buf1, de_len);
				} else if (p_info->encrypt == 0x01) {
					out_len = mfptp_idea_ecb_encrypt_evp(p_info->m_key.key, 16, buf, buf1, de_len);
				} else {
					out_len = de_len;
					memcpy(buf1, buf, de_len);
				}

				if (more == 0) {
					unsigned char *p = (unsigned char *)&out_len;

					if (out_len < 255) {
						*(ptr + offset) = 0x00;
						PTR_MOVE(1);
						*(ptr + offset) = out_len;
						PTR_MOVE(1);
						memcpy(ptr + offset, buf1, out_len);
						PTR_MOVE(out_len);
					} else if (out_len <= 65535) {
						*(ptr + offset) = 0x01;	/* 设置FP_CONTROL  */
						PTR_MOVE(1);
						*(ptr + offset) = *(p + 1);
						PTR_MOVE(1);
						*(ptr + offset) = *(p);
						PTR_MOVE(1);
						memcpy(ptr + offset, buf1, out_len);
						PTR_MOVE(out_len);
					} else {
						*(ptr + offset) = 0x02;	/* 设置FP_CONTROL  */
						PTR_MOVE(1);
						*(ptr + offset) = *(p + 2);
						PTR_MOVE(1);
						*(ptr + offset) = *(p + 1);
						PTR_MOVE(1);
						*(ptr + offset) = *(p);
						PTR_MOVE(1);
						memcpy(ptr + offset, buf1, out_len);
						PTR_MOVE(out_len);
					}

					p_info->packages--;

					if (p_info->packages == 0) {
						free(buf);
						free(buf1);
						p_info->index = p_info->doptr;
						p_info->doptr = 0;
						p_info->step = 0;
						return MFPTP_PACK_OVER;
					} else {
						free(buf);
						free(buf1);
						return MFPTP_PACK_PACKAGE;
					}
				} else {
					unsigned char *p = (unsigned char *)&out_len;

					if (out_len < 255) {
						*(ptr + offset) = 0x04;
						PTR_MOVE(1);
						*(ptr + offset) = out_len;
						PTR_MOVE(1);
						memcpy(ptr + offset, buf1, out_len);
						PTR_MOVE(out_len);
					} else if (out_len <= 65535) {
						*(ptr + offset) = 0x04;
						*(ptr + offset) = (*(ptr + offset)) | 0x01;	/* 设置FP_CONTROL  */
						PTR_MOVE(1);
						*(ptr + offset) = *(p + 1);
						PTR_MOVE(1);
						*(ptr + offset) = *(p);
						PTR_MOVE(1);
						memcpy(ptr + offset, buf1, out_len);
						PTR_MOVE(out_len);
					} else {
						*(ptr + offset) = 0x04;
						*(ptr + offset) = (*(ptr + offset)) | 0x02;	/* 设置FP_CONTROL  */
						PTR_MOVE(1);
						*(ptr + offset) = *(p + 2);
						PTR_MOVE(1);
						*(ptr + offset) = *(p + 1);
						PTR_MOVE(1);
						*(ptr + offset) = *(p);
						PTR_MOVE(1);
						memcpy(ptr + offset, buf1, out_len);
						PTR_MOVE(out_len);
					}

					free(buf);
					free(buf1);
					return MFPTP_PACK_PACKAGE;
				}
			}

			case    MFPTP_PACK_OVER:
				free(buf);
				free(buf1);
				return MFPTP_PACK_OVER;

			default:
				free(buf);
				free(buf1);
				return MFPTP_PACK_ERROR;
		}
	}
}

/*以下函数返回打包的字节数，偏移需使用者计算*/

int mfptp_pack_hdr(char *dst, struct mfptp_pack *p_info)
{
	short   ver;
	uint8_t encry;
	/* 计算加密压缩控制位  */
	uint8_t p_compress = p_info->compress << 4;

	p_compress |= 0x0f;
	encry = p_info->encrypt | 0xf0;
	p_compress &= encry;

	/* 计算版本控制位 */
	uint8_t p_version = ((uint8_t)p_info->version) << 4;
	p_version |= 0x0f;
	ver = p_info->sub_version | 0xf0;
	p_version &= (uint8_t)ver;

	dst[0] = '#';
	dst[1] = 'M';
	dst[2] = 'F';
	dst[3] = 'P';
	dst[4] = 'T';
	dst[5] = 'P';
	dst[6] = p_version;		/* 版本*/
	dst[7] = p_compress;		/* 压缩、加密*/
	dst[8] = p_info->method;	/* PUSH */
	dst[9] = p_info->packages;	/* 包个数*/

	/* 当前头部长度为10,魔鬼数字*/
	return 10;
}

int mfptp_pack_frame(char *src, int len, char *dst, int more, struct mfptp_pack *p_info)
{
	int     ret = -1;
	int     de_len;
	int     out_len;
	char    *buf = NULL;
	char    *buf1 = NULL;

#define MFPTP_FRAME_MAX_LEN (1024 * 1024 * 6)

	if (1) {
		buf1 = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == buf1) {
			printf("加密内存分配错误\n");
			return MFPTP_PACK_ERROR;
		}
	}

	if (1) {
		buf = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == buf) {
			printf("压缩内存分配错误\n");
			free(buf1);
			return MFPTP_PACK_ERROR;
		}
	}

	if (p_info->compress == 0x02) {
		de_len = mfptp_gzip(buf, MFPTP_FRAME_MAX_LEN, src, len);
	} else if (p_info->compress == 0x01) {
		de_len = mfptp_zip(buf, MFPTP_FRAME_MAX_LEN, src, len);
	} else {
		de_len = len;
		memcpy(buf, src, len);
	}

	if (p_info->encrypt == 0x02) {
		out_len = mfptp_aes_ecb_encrypt_evp(p_info->m_key.key, 16, buf, buf1, de_len);
	} else if (p_info->encrypt == 0x01) {
		out_len = mfptp_idea_ecb_encrypt_evp(p_info->m_key.key, 16, buf, buf1, de_len);
	} else {
		out_len = de_len;
		memcpy(buf1, buf, de_len);
	}

	if (0 != more) {
		dst[0] = 0x04;	/* 设置FP_CONTROL  */
	} else {
		dst[0] = 0x00;	/* 设置FP_CONTROL */
	}

	unsigned char *p = (unsigned char *)&out_len;

	if (out_len < 255) {
		dst[1] = out_len;
		memcpy(dst + 2, buf1, out_len);
		ret = out_len + 2;
	} else if (out_len <= 65535) {
		dst[0] = dst[0] | 0x01;
		dst[1] = *(p + 1);
		dst[2] = *(p);
		memcpy(dst + 3, buf1, out_len);
		ret = out_len + 3;
	} else {
		dst[0] = dst[0] | 0x02;	/* FP_CONTROL  */
		dst[1] = *(p + 2);
		dst[2] = *(p + 1);
		dst[3] = *(p);
		memcpy(dst + 4, buf1, out_len);
		ret = out_len + 4;
	}

	return ret;
}

