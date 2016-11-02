/* 文件: mfptp_callback.c
 *     版权:
 *     描述: 本文件提供mfptp_parse 函数用到的回调函数
 *     历史: 2015/3/12 新文件建立by l00167671/luokaihui
 */
#include <zmq.h>
#include "mfptp_def.h"
#include "mfptp_parser.h"
#include <stdio.h>
#include <openssl/aes.h>

/* 名      称: mfptp_drift_out_func
 * 功      能: 把mfptp 协议中的消息转化成ZMQ消息转发出去
 * 参      数:
 * 返回值:
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
void *mfptp_drift_out_callback(void *data)
{
	struct user_info                *me = (struct user_info *)data;
	struct mfptp_parser_info        *pparser = &(me->mfptp_info.parser);
	struct mfptp_status             *p_status = &(pparser->status);
	int                             cnt = p_status->package.frames;
	int                             data_size = 0;
	int                             i = 0;

	LOG(LOG_NET_UPLINK, M, "%s:uplink  pack zmq module start！\n", me->who);
	LOG(LOG_NET_UPLINK, D, "%s:drift_out going %d\n", me->who, cnt);
	x_printf(D, "drift_out going %d\n", cnt);
	int     ret;
	int     de_len;
	int     out_len;
	char    *buf = NULL;
	char    *buf1 = NULL;

	/* 单帧最大长度，需要在协议中作出约定 */
#define MFPTP_FRAME_MAX_LEN (1024 * 1024 * 6)

	// if(pparser->parser.compress){
	if (1) {
		buf1 = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == buf1) {
			LOG(LOG_NET_UPLINK, D, "%s:malloc mem error\n", me->who);
			x_printf(D, "内存分配错误\n");
			goto FAIL;
		}
	}

	if (1) {
		buf = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == buf) {
			LOG(LOG_NET_UPLINK, D, "%s:malloc mem error\n", me->who);
			x_printf(D, "内存分配错误\n");
			free(buf1);
			goto FAIL;
		}
	}

	for (i = 0; i < cnt; i++) {
		int snd_flg;
		LOG(LOG_NET_UPLINK, D, "%s:drift_out frame %d,len: %d\n", me->who, i, p_status->package.dsizes[i]);
		x_printf(D, "drift_out frame %d %d\n", i, p_status->package.dsizes[i]);

		if (i == cnt - 1) {
			LOG(LOG_NET_UPLINK, D, "%s:drift_out______ %s\n", me->who, "snd_flg = 0");
			x_printf(D, "drift_out______ %s\n", "snd_flg = 0");
			snd_flg = 0;
		} else {
			LOG(LOG_NET_UPLINK, D, "%s:drift_out______ %s\n", me->who, "snd_flg = ZMQ_SNDMORE");
			x_printf(D, "drift_out______ %s\n", "snd_flg = ZMQ_SNDMORE");
			snd_flg = ZMQ_SNDMORE;
		}

		save_frame(i, p_status->package.ptrs[i], p_status->package.dsizes[i]);
		out_len = p_status->package.dsizes[i];

		if (IDEA_ENCRYPTION == pparser->parser.encrypt) {
			mfptp_idea_ecb_decrypt(me->key, p_status->package.ptrs[i], buf, out_len);

			/* IDEA PADING算法: 不够8字节的补齐8字节,够8字节的补8字节
			 * 填充算法: m = n%8; 如果m为零则填充8个8,否则填充m个m
			 * 去填充算法: 取最后字节的值为m,若m为8，则丢弃最后8个字节
			 *             若m在[1,7]中则，丢弃m个字节
			 *             其他情况都是出错
			 */
			if (8 == buf[out_len - 1]) {
				out_len -= 8;
			} else if ((buf[out_len - 1] <= 7) && (buf[out_len - 1] >= 1)) {
				out_len -= buf[out_len - 1];
			} else {
				LOG(LOG_NET_UPLINK, D, "%s:IDEA_ENCRYPTION  error!\n", me->who);
				x_printf(D, "IDEA 解包错误\n");
				free(buf);
				free(buf1);
				goto FAIL;
			}
		} else if (AES_ENCRYPTION == pparser->parser.encrypt) {
			/* AES解密 */
			out_len = mfptp_aes_ecb_decrypt_evp(me->key, 16, p_status->package.ptrs[i], buf, p_status->package.dsizes[i]);
		} else {
			LOG(LOG_NET_UPLINK, D, "%s:frame addr = %p, frame len = %d\n", me->who, p_status->package.ptrs[i], p_status->package.dsizes[i]);
			x_printf(D, "帧地址=%p,帧长度=%d\n", p_status->package.ptrs[i], p_status->package.dsizes[i]);
			memcpy(buf, p_status->package.ptrs[i], p_status->package.dsizes[i]);
		}

		if (ZIP_COMPRESSION == pparser->parser.compress) {
			/* zip解密，本zip即是zlib的标准方式 */
			de_len = mfptp_unzip(buf, out_len, buf1, MFPTP_FRAME_MAX_LEN);

			if (de_len > 0) {
				ret = mfptp_zmq_push_frame(me->processor->zmq_socket, buf1, de_len, snd_flg);

				if (FALSE == ret) {
					LOG(LOG_NET_UPLINK, D, "%s:push frame ------------error\n", me->who);
					x_printf(D, "push frame -------------错误\n");
				} else {
					// x_printf(D, "push frame -------------OK\n");
					LOG(LOG_NET_UPLINK, D, "%s:push frame --zip compress-----de_len=%d------OK\n", me->who, de_len);
					x_printf(D, "push frame --compress压缩-----de_len=%d------OK\n", de_len);
				}

				mfptp_log(me->who, buf1, de_len, 1);
			}
		} else if (GZIP_COMPRESSION == pparser->parser.compress) {
			/* gz解密, 本gz即是zlib的gz方式 */
			de_len = mfptp_ungzip(buf, out_len, buf1, MFPTP_FRAME_MAX_LEN);

			if (de_len > 0) {
				ret = mfptp_zmq_push_frame(me->processor->zmq_socket, buf1, de_len, snd_flg);

				if (FALSE == ret) {
					LOG(LOG_NET_UPLINK, D, "%s:push frame -------------error!\n", me->who);
					x_printf(D, "push frame -------------错误\n");
				} else {
					// x_printf(D, "push frame -------------OK\n");
					LOG(LOG_NET_UPLINK, D, "%s:push frame --gz compress-----de_len=%d------OK\n", me->who, de_len);
					x_printf(D, "push frame --gz压缩-----de_len=%d------OK\n", de_len);
				}

				mfptp_log(me->who, buf1, de_len, 1);
			}
		} else {
			ret = mfptp_zmq_push_frame(me->processor->zmq_socket, buf, out_len, snd_flg);

			if (FALSE == ret) {
				LOG(LOG_NET_UPLINK, D, "%s:push frame ------------error!\n", me->who);
				x_printf(D, "push frame -------------错误\n");
			} else {
				LOG(LOG_NET_UPLINK, D, "%s:push frame --no compress-----out_len=%d------OK\n", me->who, out_len);
				x_printf(D, "push frame --非压缩-----out_len=%d------OK\n", out_len);
			}

			mfptp_log(me->who, buf, out_len, 1);
		}
	}

	free(buf1);
	free(buf);

	LOG(LOG_NET_UPLINK, M, "%s:uplink  pack zmq moudle OK\n\n", me->who);
	return NULL;

FAIL:
	LOG(LOG_NET_UPLINK, M, "%s:uplink  pack zmq moudle failed\n\n", me->who);
	return NULL;
}

/* 名      称: mfptp_auth_func
 * 功      能: 对于登录消息进行验证
 * 参      数:
 * 返回值:
 * 修      改: 新生成函数l00167671 at 2015/2/28, 暂不进行任何验证
 */
void *mfptp_auth_callback(void *data)
{
	struct user_info                *me = (struct user_info *)data;
	struct mfptp_parser_info        *pparser = &(me->mfptp_info.parser);
	struct mfptp_status             *p_status = &(pparser->status);
	int                             cnt = p_status->package.frames;

	/* 根据约定登陆消息只有一个帧*/
	if (1 == cnt) {
		if ((p_status->package.dsizes[0] > 0) && (p_status->package.dsizes[0] < MFPTP_UID_MAX_LEN)) {
			/* 前15个字节是IMEI，最后1个字节是是否续传的标志 */
			memcpy(me->who, p_status->package.ptrs[0], p_status->package.dsizes[0] - 1);
			me->force_login = (int)*(p_status->package.ptrs[0] + 15);
			LOG(LOG_MFPTP_AUTH, D, "user %s  force_login=%d (0 continution，1 non-continution)\n", me->who, me->force_login);
			x_printf(D, "force_login=%d\n", me->force_login);
		} else {
			/* 用户标识长度错误 */
			memcpy(me->who, MFPTP_INVALID_UID, MFPTP_INVALID_UID_LEN);
		}
	} else {
		/* 登陆消息帧数错误*/
		memcpy(me->who, MFPTP_INVALID_UID, MFPTP_INVALID_UID_LEN);
	}

	return NULL;
}

/* 测试函数发布时要删除 */
void save_frame(int i, char *buf, int len)
{
	FILE    *fp;
	char    file[256];
	int     ret;

	sprintf(file, "frame%d", i);
	fp = fopen(file, "w+");
	ret = fwrite(buf, len, 1, fp);
	fclose(fp);
}

