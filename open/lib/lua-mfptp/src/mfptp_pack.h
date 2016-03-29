#ifndef __MFPTP_PACK_H__
#define __MFPTP_PACK_H__
#include <string.h>

/* 名      称: mfptp_pack_hdr
 * 功      能: 序列化一个mfptp协议头到缓冲区中
 * 参      数: dst,添加包头后的数据写入到该地址
 *               ver,版本号more
 *               sk_type,socket类型标识数据类型
 *               pkt_cnt,数据包的个数
 * 返  回  值: 序列化头部后的长度
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_pack_hdr(char *dst, int ver, int sk_type, int pkt_cnt);

/* 名      称: mfptp_pack_frame
 * 功      能: 序列化一个mfptp协议帧到缓冲区中
 * 参      数: src,数据指针，要打包的数据的首地址
 *             len,源数据的长度
 *             dst,添加包头后的数据写入到该地址
 *               more,是否还有后续帧，0有，1无
 * 返  回  值: 序列化后帧的长度
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_pack_frame(char *src, int len, char *dst, int more);
#endif

