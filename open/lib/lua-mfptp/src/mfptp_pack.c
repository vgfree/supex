#include "mfptp_pack.h"

/* 名      称: mfptp_pack_hdr
 * 功      能: 序列化一个mfptp协议头到缓冲区中
 * 参      数: dst,添加包头后的数据写入到该地址
 *               ver,版本号more
 *               sk_type,socket类型标识数据类型
 *               pkt_cnt,数据包的个数
 * 返  回  值: 序列化头部后的长度
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_pack_hdr(char *dst, int ver, int sk_type, int pkt_cnt)
{
	dst[0] = '#';
	dst[1] = 'M';
	dst[2] = 'F';
	dst[3] = 'P';
	dst[4] = 'T';
	dst[5] = 'P';
	dst[6] = ver;		/* 版本*/
	dst[7] = 0x00;		/* 压缩、加密*/
	dst[8] = sk_type;	/* PUSH */
	dst[9] = pkt_cnt;	/* 包个数*/

	/* 当前头部长度为10,魔鬼数字*/
	return 10;
}

/* 名      称: mfptp_pack_frame
 * 功      能: 序列化一个mfptp协议帧到缓冲区中
 * 参      数: src,数据指针，要打包的数据的首地址
 *             len,源数据的长度
 *             dst,添加包头后的数据写入到该地址
 *               more,是否还有后续帧，0无，大于0有
 * 返  回  值: 序列化后帧的长度
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_pack_frame(char *src, int len, char *dst, int more)
{
	int ret = -1;

	if (0 != more) {
		dst[0] = 0x04;	/* 设置FP_CONTROL  */
	} else {
		dst[0] = 0x00;	/* 设置FP_CONTROL */
	}

	unsigned char *p = (unsigned char *)&len;

	if (len <= 255) {
		dst[1] = len;
		memcpy(dst + 2, src, len);
		ret = len + 2;
	} else if (len <= 65535) {
		dst[0] = dst[0] | 0x01;	/* FP_CONTROL  */
		dst[1] = *(p + 1);
		dst[2] = *(p);
		memcpy(dst + 3, src, len);
		ret = len + 3;
	} else {
		dst[0] = dst[0] | 0x02;	/* FP_CONTROL  */
		dst[1] = *(p + 2);
		dst[2] = *(p + 1);
		dst[3] = *(p);
		memcpy(dst + 4, src, len);
		ret = len + 4;
	}

	return ret;
}

