/*
 *
 */

#include "mfptp_parser.h"
#include <unistd.h>

/*  FP_Control的长度 */
#define FP_CONTROL_SIZE                 (0x01)
#define FP_CONTROL_LOW01_BITS_MASK      (0x03)
#define FP_CONTROL_LOW23_BITS_MASK      (0x0C)

#define MFPTP_BURST_HEADER_LEN          (0x0A)
#define MFPTP_BURST_HEADER_START        (0x00)
#define MFPTP_BURST_HEADER_VERSION      (0x07)
#define MFPTP_BURST_HEADER_DATA_MODE    (0x08)
#define MFPTP_BURST_HEADER_SOCKET_MODE  (0x09)
#define MFPTP_BURST_HEADER_PACKAGES     (0x0A)
#define MFPTP_BURST_FRAME_START         (0x0B)

#define GET_LOW01_BITS(ctrl)    (ctrl & FP_CONTROL_LOW01_BITS_MASK)
#define GET_LOW23_BITS(ctrl)    ((ctrl & FP_CONTROL_LOW23_BITS_MASK) >> 2)

#define PTR_MOVE(len)			\
	do {				\
		offset += len;		\
		p_status->doptr += len;	\
	} while (0)

#define ELSE_RETURN(x)	  \
	else {		  \
		return x; \
	}

#define CHECK_SIZE(a, b)								     \
	do {										     \
		if ((a) + (b) > usr->recv.get_size) {					     \
			if (p_status->step == MFPTP_PARSE_PACKAGE && p_status->index > 0) {  \
				if ((a) + (b) > MAX_REQ_SIZE) {				     \
					int     left_len = MAX_REQ_SIZE - (a);		     \
					char    *left_data = ptr + offset;		     \
					char    *begin = ptr + p_parser->pos_frame_start;    \
					if (left_len <= 0) {				     \
						usr->recv.get_size = 0;			     \
						p_parser->packages -= (p_status->index - 1); \
						p_status->index = 0;			     \
						p_status->doptr -= length_over;		     \
						length_over = 0;			     \
						return p_status->step;			     \
					} else {					     \
						while (left_len--) {			     \
							printf("-%c", *left_data);	     \
							*begin++ = *left_data++;	     \
						}					     \
						usr->recv.get_size -= length_over;	     \
						p_parser->packages -= (p_status->index - 1); \
						p_status->index = 0;			     \
						p_status->doptr -= length_over;		     \
						length_over = 0;			     \
					}						     \
				}							     \
			}								     \
			return p_status->step;						     \
		}									     \
	} while (0)

/*      名    称: mfptp_pack_string
 *	功    能: 把字符串打包成mfptp格式数据,单包单帧
 *	参    数: src, 要打包的字符串首地址
 *	          buf, 输出的mfptp帧保存在这个起始地址上,空间由调用者在调用前分配
 *	          max_len, buf指向内存空间的长度,
 *	          这个长度至少应该等于包头长度+帧头长度+字符串的长度,否则打包会失败
 *	返 回 值:  -1, 失败;  0成功。
 *	修    改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_pack_string(char *src, char *buf, int max_len)
{
	int     ret = -1;
	int     i = 0;

	buf[0] = '#';
	buf[1] = 'M';
	buf[2] = 'F';
	buf[3] = 'P';
	buf[4] = 'T';
	buf[5] = 'P';
	buf[6] = 0x01;	/* 版本*/
	buf[7] = 0x00;	/*压缩、加密*/
	buf[8] = 0x04;	/* REP */
	buf[9] = 0x01;	/* 包个数*/
	buf[10] = 0x00;	/* FP_CONTROL  */

	if (NULL != src) {
		int len = strlen(src);

		if (len > 255) {
			len = 255;
		}

		buf[11] = len;
		memcpy(&buf[12], src, len);
		ret = len + 12;
	}

	x_printf(D, "发送了%d个字节\n", ret);

	for (i = 0; i < ret; i++) {
		if (isgraph(buf[i])) {
			x_printf(D, "%c ", buf[i]);
		} else {
			x_printf(D, "%x ", buf[i]);
		}
	}

	return ret;
}

/*	名       称: mfptp_pack_login_ok
 *	功	能: 生成登录认证成功的应答消息帧
 *	参	数: buf, 存储消息帧的首地址
 *	            len, 续传长度
 *	            usr, 用户
 *	返  回  值: 应答消息帧的长度
 *	修      改: 新生成函数l00167671 at 2015/5/7
 */
int mfptp_pack_login_ok(char *buf, int len, struct user_info *usr)
{
	int     ret = -1;
	int     i = 0;
	int     network_len = htonl(len);

	printf("............len=%d\n", len);
	buf[0] = '#';
	buf[1] = 'M';
	buf[2] = 'F';
	buf[3] = 'P';
	buf[4] = 'T';
	buf[5] = 'P';
	buf[6] = 0x01;	/* 版本*/
	buf[7] = 0x00;	/*压缩、加密*/
	buf[8] = 0x00;	/* PAIR_METHOD */
	buf[9] = 0x01;	/* 包个数*/
	buf[10] = 0x00;	/* FP_CONTROL  */
	buf[11] = 0x01;	/* 帧长度 */
	buf[12] = 0x01;	/* 成功 */

	/* 续传长度 */
	memcpy((void *)&buf[13], (void *)&network_len, sizeof(network_len));

#define  SECRET_KEY_LEN         (16)
#define  PACKAGE_HDR_LEN        (10)

	/* 密钥 */
	memcpy((void *)&buf[17], usr->key, SECRET_KEY_LEN);

	/* 响应帧的长度 */
	buf[11] = 0x01 + sizeof(network_len) + SECRET_KEY_LEN;

	/* 包头长度+帧头长度+成功标识长度+续传数据位置长度+密钥长度*/
	return PACKAGE_HDR_LEN + 2 + 1 + sizeof(network_len) + SECRET_KEY_LEN;
}

/*	名       称: mfptp_register_callback
 *	功	能: 设置回调函数
 *	参	数:
 *	返  回  值: 无
 *	修      改: 新生成函数l00167671 at 2015/2/28
 */
void mfptp_register_callback(struct user_info *usr, mfptp_callback_func fun)
{
	usr->mfptp_info.parser.func = fun;
}

void mfptp_init_parser_info(struct mfptp_parser_info *p_info)
{
	p_info->parser.compress = 0;
	p_info->parser.encrypt = 0;
	p_info->parser.func = NULL;
	p_info->parser.method = 0;
	p_info->parser.packages = 0;
	p_info->parser.pos_frame_start = 0;
	p_info->parser.sub_version = 0;
	p_info->parser.version = 0;
	p_info->status.index = 0;
	p_info->status.package.complete = true;
	p_info->status.package.frames = 0;
	p_info->status.step = 0;
}

int mfptp_parse(struct user_info *usr)
{
	assert(usr);
	assert(usr->recv.buf_addr);
	struct mfptp_parser     *p_parser = &(usr->mfptp_info.parser);
	struct mfptp_status     *p_status = &(usr->mfptp_info.status);

	int offset = p_status->doptr;

	/* 当前处理到的地址*/
	unsigned char *ptr = usr->recv.buf_addr;

	/* frame长度所占用的字节数*/
	int size_f_size = 0;

	/* 数据长度*/
	int frame_size = 0;

	int i = 0;

	/* 分析过的帧所包含数据的长度*/
	int length_over = 0;

	while (true) {
		switch (p_status->step)
		{
			case    MFPTP_PARSE_ILEGAL:
			case    MFPTP_PARSE_INIT:
			case    MFPTP_PARSE_HEAD:
			{
				LOG(LOG_MFPTP_PARSE, M, "%s:mfptp parse moudle start, one request ......\n", usr->who);
				LOG(LOG_MFPTP_PARSE, D, "%s:mfptp parse  header start!\n", usr->who);

				CHECK_SIZE(offset, MFPTP_BURST_HEADER_LEN);

				// if (MFPTP_BURST_HEADER_START == offset){
				if (1) {
					/* 包的前6个字节必须是"#MFPTP" */
					if (strncmp(ptr + offset, "#MFPTP", 6) != 0) {
						return MFPTP_PARSE_ILEGAL;
					}

					PTR_MOVE(6);

					/* 版本信息 */
					p_parser->version = (uint8_t)(*(ptr + offset) >> 4);
					p_parser->sub_version = (uint8_t)(*(ptr + offset) << 4);
					p_parser->sub_version = p_parser->sub_version >> 4;
					PTR_MOVE(1);

					/* 加密和压缩信息 */
					p_parser->compress = (uint8_t)(*(ptr + offset) >> 4);
					p_parser->encrypt = (uint8_t)(*(ptr + offset) << 4);
					p_parser->encrypt = p_parser->encrypt >> 4;
					PTR_MOVE(1);

					/* 方法信息 */
					p_parser->method = (uint8_t)(*(ptr + offset));
					PTR_MOVE(1);

					if ((p_parser->compress > 0x3) || (p_parser->encrypt > 0x3)
						|| (p_parser->method > 0x9)) {
						x_printf(E, "异常啊\n");
						LOG(LOG_MFPTP_PARSE, E, "%s: compress , encrypt or method error! \n", usr->who);
					}

					if (p_parser->method == HEARTBEAT_METHOD) {
						PTR_MOVE(1);
						x_printf(D, "心跳\n");
						LOG(LOG_MFPTP_PARSE, D, "%s:heart package\n", usr->who);
						return MFPTP_PARSE_OVER;
					}

					/* 包数信息 */
					p_parser->packages = (uint8_t)(*(ptr + offset));

					if (p_parser->packages == 0) {
						LOG(LOG_MFPTP_PARSE, E, "%s:there is no data \n", usr->who);
						return MFPTP_PARSE_NODATA;
					}

					PTR_MOVE(1);
				}

				ELSE_RETURN(MFPTP_PARSE_HEAD);
				p_status->step = MFPTP_PARSE_PACKAGE;	/*-parse head end-*/
				p_parser->pos_frame_start = offset;
				p_status->package.frames = 0;

				LOG(LOG_MFPTP_PARSE, D, "%s:mfptp parse  header over!\n", usr->who);

				break;
			}

			case    MFPTP_PARSE_PACKAGE:
			{
				LOG(LOG_MFPTP_PARSE, D, "%s:mfptp parse  package body start\n", usr->who);
				LOG(LOG_MFPTP_PARSE, D, "%s:mfptp parse  a frame start\n", usr->who);
				/* 至少保证当前有FP_CONTROL字段*/
				CHECK_SIZE(offset, 1);

				/* FP_control 字段*/
				uint8_t fp_control = (uint8_t)(*(ptr + offset));

				/* fp_control的最低2位表示f_size字段占据的长度*/
				size_f_size = GET_LOW01_BITS(fp_control) + 1;

				/* fp_control的第2、3位表示package是否结束,只有0和1两个取值*/
				p_status->package.complete = GET_LOW23_BITS(fp_control);

				/* 取值超出协议允许*/
				if (p_status->package.complete > 1) {
					return MFPTP_PARSE_ILEGAL;
				}

				CHECK_SIZE(offset, 1 + size_f_size);

				/* 取出数据长度，本长度低地址字节表示高位*/
				frame_size = 0;

				for (i = 0; i < size_f_size; i++) {
					frame_size += *(ptr + i + offset + 1) << (8 * (size_f_size - 1 - i));
				}

				CHECK_SIZE(offset, size_f_size + frame_size);

				p_status->package.frames++;

				if (p_status->package.frames > MAX_FRAMES_PER_PACKAGE) {
					x_printf(D, "帧数太多了，出错\n");
					LOG(LOG_MFPTP_PARSE, E, "%s:error ,too many frames!\n", usr->who);
					PTR_MOVE(1 + size_f_size + frame_size);
					return MFPTP_PARSE_MOREDATA;
				}

				if (p_parser->method != HEARTBEAT_METHOD) {
					p_status->package.ptrs[p_status->package.frames - 1] = ptr + offset + size_f_size + 1;
					p_status->package.dsizes[p_status->package.frames - 1] = frame_size;
				} else {
					p_status->package.frames--;
				}

				length_over += (FP_CONTROL_SIZE + size_f_size + frame_size);
				LOG(LOG_MFPTP_PARSE, D, "%s:mfptp parse  a frame over\n", usr->who);

				/* package最后帧处理完毕 */
				if (p_status->package.complete == 0) {
					LOG(LOG_MFPTP_PARSE, D, "%s:mfptp parse  a package over\n", usr->who);
					p_status->index++;

					/*-接收完成一个包后调用回调函数，心跳包直接丢弃,不调用回调函数-*/
					if (p_parser->func && (p_parser->method != HEARTBEAT_METHOD)) {
						p_parser->func(usr);
					}

					/* 一次请求结束, 一次请求有多个package组成,每个package由多个frame组成 */
					if (p_status->index == p_parser->packages) {
						PTR_MOVE(FP_CONTROL_SIZE + size_f_size + frame_size);
						int len = usr->recv.get_size - p_status->doptr;

						x_printf(D, "缓冲中还剩余len = %d \n", len);
						LOG(LOG_MFPTP_PARSE, D, "%s:one request data is parsed,there is  (%d bytes)data in net cache ! \n", usr->who, len);

						if (len > usr->recv.get_size) {
							x_printf(D, "缓冲中还剩余大于get_size %d\n", usr->recv.get_size);
							LOG(LOG_MFPTP_PARSE, D, "%s:one request data is parsed, the data in net cache is more than get_size( %d bytes)\n", usr->who, usr->recv.get_size);
						}

						if (len > 0) {
							memcpy(usr->recv.buf_addr, usr->recv.buf_addr + p_status->doptr, len);
							usr->recv.get_size = len;
							p_status->doptr = 0;
						} else if (0 == len) {
							usr->recv.get_size = 0;
							p_status->doptr = 0;
						}

						return MFPTP_PARSE_OVER;
					} else {
						/*  package控制结构体清空*/
						memset(&(p_status->package), 0, sizeof(struct mfptp_package));
					}
				}

				/* package还有后续frame */
				PTR_MOVE(FP_CONTROL_SIZE + size_f_size + frame_size);
				frame_size = 0;

				break;
			}

			case    MFPTP_PARSE_OVER:
				return MFPTP_PARSE_OVER;

			default:
				break;
		}
	}
}

/* 名      称: mfptp_pack_frames_with_hdr
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

/* 名      称: mfptp_pack_frames_with_hdr
 * 功      能: 把src指向的长度为len的数据打包成符合mfptp协议的数据包
 * 参      数: src, 指向要打包的数据; len, 要打包数据的长度;
 *               dst,添加包头后的数据写入到该地址
 *               more,非零表示有后续帧，零表示无后续帧
 * 返  回  值: 序列化后的数据长度
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int mfptp_pack_frames_with_hdr(char *src, int len, char *dst, int more)
{
	int ret = -1;

	dst[0] = '#';
	dst[1] = 'M';
	dst[2] = 'F';
	dst[3] = 'P';
	dst[4] = 'T';
	dst[5] = 'P';
	dst[6] = 0x01;	/* 版本*/
	dst[7] = 0x00;	/* 压缩、加密*/
	dst[8] = 0x08;	/* PUSH */
	dst[9] = 0x1;	/* 包个数*/

	if (0 != more) {
		dst[10] = 0x04;	/* FP_CONTROL  */
	} else {
		dst[10] = 0x00;
	}

	unsigned char *p = (unsigned char *)&len;

	if (len <= 255) {
		dst[11] = len;
		memcpy(dst + 12, src, len);
		ret = len + 12;
	} else if (len <= 65535) {
		dst[11] = *(p + 1);
		dst[12] = *(p);
		dst[10] = dst[10] | 0x01;	/* 设置FP_CONTROL  */
		memcpy(dst + 13, src, len);
		ret = len + 13;
	} else {
		dst[11] = *(p + 2);
		dst[12] = *(p + 1);
		dst[13] = *(p);
		dst[10] = dst[10] | 0x02;	/* 设置FP_CONTROL  */
		memcpy(dst + 14, src, len);
		ret = len + 14;
	}

	return ret;
}

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

