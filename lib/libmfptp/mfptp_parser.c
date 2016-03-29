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

#define PTR_MOVE(len)			     \
	do {				     \
		offset += len;		     \
		p_info->status.doptr += len; \
	} while (0)

#define ELSE_RETURN(x)	  \
	else {		  \
		return x; \
	}

#define CHECK_SIZE(a, b)									       \
	do {											       \
		if ((a) + (b) > get_size) {							       \
			if (p_info->status.step == MFPTP_PARSE_PACKAGE && p_info->status.index > 0) {  \
				if ((a) + (b) > MAX_REQ_SIZE) {					       \
					int     left_len = MAX_REQ_SIZE - (a);			       \
					char    *left_data = ptr + offset;			       \
					char    *begin = ptr + p_info->parser.pos_frame_start;	       \
					if (left_len <= 0) {					       \
						get_size = 0;					       \
						p_info->parser.packages -= (p_info->status.index - 1); \
						p_info->status.index = 0;			       \
						p_info->status.doptr -= length_over;		       \
						length_over = 0;				       \
						return p_info->status.step;			       \
					} else {						       \
						while (left_len--) {				       \
							printf("-%c", *left_data);		       \
							*begin++ = *left_data++;		       \
						}						       \
						get_size -= length_over;			       \
						p_info->parser.packages -= (p_info->status.index - 1); \
						p_info->status.index = 0;			       \
						p_info->status.doptr -= length_over;		       \
						length_over = 0;				       \
					}							       \
				}								       \
			}									       \
			return p_info->status.step;						       \
		}										       \
	} while (0)

/*
 *
 * #define CHECK_SIZE(a,b) do{    \
 \
 \        if ( (a) + (b) > get_size ){	\
 \                if (p_info->status.step == MFPTP_PARSE_PACKAGE && p_info->status.index > 0){ \
 \                         printf("----------p_info->status.index > 0-----------");              \
 \                }	\
 \                return p_info->status.step;\
 \        }\
 \   }while(0)
 \
 */
int mfptp_send_callback(struct mfptp_parser_info *p_info, mfptp_send fun)
{
	p_info->p_send = fun;
}

void mfptp_register_callback(struct mfptp_parser_info *p_info, mfptp_callback_func fun)
{
	p_info->parser.func = fun;
}

/* 名  称: mfptp_init_parser_info;
 * 功  能: 初始化结构体变量;
 * 参  数: p_info指针指向mfptp_parser_info结构体;
 * 返回值: 无
 * 作  者: chenliuliu    at 2015/10/15
 */
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
	p_info->status.doptr = 0;
}

/* 名  称: mfptp_parse;
 * 功  能: 将接收到的数据以mfptp协议格式解析;
 * 参  数: buf_addr,指向要解析的数据;get_size 接收的数据长度;
 *         p_info指针指向mfptp_parser_info结构体,内含控制变量;
 * 返回值: MFPTP_PARSE_ILEGAL   包头不正确或者控制帧信息不符和协议;
 *         MFPTP_PARSE_NODATA   包头后无数据;
 *         MFPTP_PARSE_MOREDATA 帧数超出协议;
 *         MFPTP_PARSE_OVER     解析结束;
 * 作  者: chenliuliu    at 2015/10/15
 */
int mfptp_parse(uint8_t *buf_addr, int get_size, struct mfptp_parser_info *p_info)
{
	assert(buf_addr);
	int offset = p_info->status.doptr;

	/* 当前处理到的地址*/
	unsigned char *ptr = buf_addr;

	/* frame长度所占用的字节数*/
	int size_f_size = 0;

	/* 数据长度*/
	int frame_size = 0;

	int i = 0;

	/* 分析过的帧所包含数据的长度*/
	int length_over = 0;

	while (true) {
		switch (p_info->status.step)
		{
			case    MFPTP_PARSE_ILEGAL:

			case    MFPTP_PARSE_INIT:
			case    MFPTP_PARSE_HEAD:
			{
				CHECK_SIZE(offset, MFPTP_BURST_HEADER_LEN);

				if (1) {
					/* 包的前6个字节必须是"#MFPTP" */
					if (strncmp(ptr + offset, "#MFPTP", 6) != 0) {
						return MFPTP_PARSE_ILEGAL;
					}

					PTR_MOVE(6);

					/* 版本信息 */

					p_info->parser.version = (unsigned char)*(ptr + offset) >> 4;
					p_info->parser.sub_version = (unsigned char)*(ptr + offset) << 4;
					p_info->parser.sub_version = p_info->parser.sub_version >> 4;
					PTR_MOVE(1);

					/* 加密和压缩信息 */
					p_info->parser.compress = (uint8_t)(*(ptr + offset) >> 4);

					p_info->parser.encrypt = (uint8_t)(*(ptr + offset) << 4);
					p_info->parser.encrypt = p_info->parser.encrypt >> 4;

					PTR_MOVE(1);

					/* 方法信息 */
					p_info->parser.method = (uint8_t)(*(ptr + offset));
					PTR_MOVE(1);

					if ((p_info->parser.compress > 0x2) || (p_info->parser.encrypt > 0x2)
						|| (p_info->parser.method > 0x9)) {
						printf("异常啊,compress , encrypt or method error!\n");
					}

					if (p_info->parser.method == HEARTBEAT_METHOD) {
						PTR_MOVE(1);
						printf("心跳包\n");
						return MFPTP_PARSE_OVER;
					}

					/* 包数信息 */
					p_info->parser.packages = (uint8_t)(*(ptr + offset));

					if (p_info->parser.packages == 0) {
						printf("there is no data\n");
						return MFPTP_PARSE_NODATA;
					}

					PTR_MOVE(1);
				}

				ELSE_RETURN(MFPTP_PARSE_HEAD);
				p_info->status.step = MFPTP_PARSE_PACKAGE;	/*-parse head end-*/
				p_info->parser.pos_frame_start = offset;
				p_info->status.package.frames = 0;

				break;
			}

			case    MFPTP_PARSE_PACKAGE:
			{
				/* 至少保证当前有FP_CONTROL字段*/
				CHECK_SIZE(offset, 1);

				/* FP_control 字段*/
				uint8_t fp_control = (uint8_t)(*(ptr + offset));

				/* fp_control的最低2位表示f_size字段占据的长度*/
				size_f_size = GET_LOW01_BITS(fp_control) + 1;

				/* fp_control的第2、3位表示package是否结束,只有0和1两个取值*/
				p_info->status.package.complete = GET_LOW23_BITS(fp_control);

				/* 取值超出协议允许*/
				if (p_info->status.package.complete > 1) {
					return MFPTP_PARSE_ILEGAL;
				}

				/* 取出数据长度，本长度低地址字节表示高位*/
				frame_size = 0;

				for (i = 0; i < size_f_size; i++) {
					frame_size += *(ptr + i + offset + 1) << (8 * (size_f_size - 1 - i));
				}

				CHECK_SIZE(offset, size_f_size + frame_size);

				p_info->status.package.frames++;

				if (p_info->status.package.frames > MAX_FRAMES_PER_PACKAGE) {
					printf("帧数太多了，出错\n");
					PTR_MOVE(1 + size_f_size + frame_size);
					return MFPTP_PARSE_MOREDATA;
				}

				if (p_info->parser.method != HEARTBEAT_METHOD) {
					p_info->status.package.ptrs[p_info->status.package.frames - 1] = ptr + offset + size_f_size + 1;
					p_info->status.package.dsizes[p_info->status.package.frames - 1] = frame_size;
				} else {
					p_info->status.package.frames--;
				}

				length_over += (FP_CONTROL_SIZE + size_f_size + frame_size);

				/* package最后帧处理完毕 */
				if (p_info->status.package.complete == 0) {
					p_info->status.index++;

					/*-接收完成一个包后调用回调函数，心跳包直接丢弃,不调用回调函数-*/
					if (p_info->parser.func && (p_info->parser.method != HEARTBEAT_METHOD)) {
						p_info->parser.func(p_info);
					}

					/* 一次请求结束, 一次请求有多个package组成,每个package由多个frame组成 */
					if (p_info->status.index == p_info->parser.packages) {
						PTR_MOVE(FP_CONTROL_SIZE + size_f_size + frame_size);
						int len = get_size - p_info->status.doptr;

						printf("缓冲中还剩余len = %d \n", len);

						if (len > get_size) {
							printf("缓冲中还剩余大于get_size %d\n", get_size);
						}

						if (len > 0) {
							memcpy(buf_addr, buf_addr + p_info->status.doptr, len);
							get_size = len;
							p_info->status.doptr = 0;
						} else if (0 == len) {
							get_size = 0;
							p_info->status.doptr = 0;
						}

						return MFPTP_PARSE_OVER;
					} else {
						/*  package控制结构体清空*/
						memset(&(p_info->status.package), 0, sizeof(struct mfptp_package));
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

/* 名      称: mfptp_drift_out_func
 * 功      能: 把mfptp 协议中的消息转发出去
 * 参      数:
 * 返回值:
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
void *mfptp_drift_out_callback(void *data)
{
	struct mfptp_parser_info        *me = (struct mfptp_parser_info *)data;
	int                             cnt = me->status.package.frames;
	int                             data_size = 0;
	int                             i = 0;

	printf("drift_out going %d\n", cnt);
	int ret;
	me->de_len = 0;
	me->out_len = 0;
	me->buf = NULL;
	me->buf1 = NULL;

	/* 单帧最大长度，需要在协议中作出约定 */
#define MFPTP_FRAME_MAX_LEN (1024 * 1024 * 6)

	if (1) {
		me->buf1 = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == me->buf1) {
			printf("内存分配错误\n");
			goto FAIL;
		}
	}

	if (1) {
		me->buf = malloc(MFPTP_FRAME_MAX_LEN);

		if (NULL == me->buf) {
			printf("内存分配错误\n");
			free(me->buf1);
			goto FAIL;
		}
	}

	for (i = 0; i < cnt; i++) {
		me->snd_flg = 0;

		printf("drift_out frame %d %d\n", i, me->status.package.dsizes[i]);

		if (i == cnt - 1) {
			printf("drift_out______ %s\n", "me->snd_flg = 0");
			me->snd_flg = 0;
		} else {
			printf("drift_out______ %s\n", "me->snd_flg = 1");
			me->snd_flg = 1;
		}

		printf("me->status.package.dsizes[i] = %d\n", me->status.package.dsizes[i]);

		if (IDEA_ENCRYPTION == me->parser.encrypt) {
			me->out_len = mfptp_idea_ecb_decrypt_evp(me->m_key.key, 16, me->status.package.ptrs[i], me->buf, me->status.package.dsizes[i]);
		} else if (AES_ENCRYPTION == me->parser.encrypt) {
			/* AES解密 */

			me->out_len = mfptp_aes_ecb_decrypt_evp(me->m_key.key, 16, me->status.package.ptrs[i], me->buf, me->status.package.dsizes[i]);
		} else {
			printf("帧地址=%p,帧长度=%d\n", me->status.package.ptrs[i], me->status.package.dsizes[i]);
			memcpy(me->buf, me->status.package.ptrs[i], me->status.package.dsizes[i]);
			me->out_len = me->status.package.dsizes[i];
		}

		if (ZIP_COMPRESSION == me->parser.compress) {
			/* zip解密，本zip即是zlib的标准方式 */
			me->de_len = mfptp_unzip(me->buf, me->out_len, me->buf1, MFPTP_FRAME_MAX_LEN);

			if (me->de_len > 0) {
				// me->p_send(data);
			}
		} else if (GZIP_COMPRESSION == me->parser.compress) {
			/* gz解密, 本gz即是zlib的gz方式 */

			me->de_len = mfptp_ungzip(me->buf, me->out_len, me->buf1, MFPTP_FRAME_MAX_LEN);

			if (me->de_len > 0) {
				//   me->p_send(data);
			}
		} else {
			memcpy(me->buf1, me->buf, me->out_len);
			me->de_len = me->out_len;

			//   me->p_send(data);
		}
	}

	free(me->buf1);
	free(me->buf);

FAIL:

	return NULL;
}

/* 名      称: mfptp_auth_func
 * 功      能: 对于登录消息进行验证
 * 参      数:
 * 返  回  值: 无
 * 修      改: chenliuliu   at 2015/10/16
 */
void *mfptp_auth_callback(void *data)
{
	struct mfptp_parser_info        *me = (struct mfptp_parser_info *)data;
	int                             cnt = me->status.package.frames;

	/* 根据约定登陆消息只有一个帧*/
	if (1 == cnt) {
		if ((me->status.package.dsizes[0] > 0) && (me->status.package.dsizes[0] < MFPTP_UID_MAX_LEN)) {
			/* 前15个字节是IMEI，最后1个字节是是否续传的标志 */
			memcpy(me->who, me->status.package.ptrs[0], me->status.package.dsizes[0] - 1);
			me->force_login = (int)*(me->status.package.ptrs[0] + 15);

			printf("force_login=%d\n", me->force_login);
		} else {
			/* 用户标识长度错误 */
			printf("用户标识长度错误\n");
			memcpy(me->who, MFPTP_INVALID_UID, MFPTP_INVALID_UID_LEN);
		}
	} else {
		/* 登陆消息帧数错误*/
		printf("登陆消息帧数错误\n");
		memcpy(me->who, MFPTP_INVALID_UID, MFPTP_INVALID_UID_LEN);
	}

	return NULL;
}

