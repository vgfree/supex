/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __MFPTP_UTILS_H__
#define __MFPTP_UTILS_H__

#include "../comm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif



#define	MFPTP_MAJOR_VERSION	1	/* MFPTP协议的主版本号 */
#define MFPTP_MINOR_VERSION	0	/* MFPTP协议的副版本号 */
#define	MFPTP_MAX_FRAMES	13	/* MFPTP协议单包支持最大的帧的数量 */
#define MFPTP_MAX_DATASIZE	4194304	/* MFPTP协议一个帧携带数据的最大值 */
#define	MFPTP_MAX_PACKAGES	8	/* MFPTP协议支持携带的最大包数 */

#define MFPTP_HEADER_LEN	10	/* MFPTP协议头的所占的字节数 */
#define MFPTP_FP_CONTROL_LEN	1	/* MFPTP协议FP_control的所占的字节数 */
#define MFPTP_F_SIZE_MINLEN	1	/* MFPTP协议F_size所占的最小字节数 */
#define MFPTP_F_SIZE_MAXLEN	4	/* MFPTP协议F_size所占的最大字节数 */

/* 检测MFPTP协议头的6个字节是否正确 */
#define CHECK_HEADER(parser)	(!strncmp(&(((*parser->ms.data)[parser->ms.dosize])), "#MFPTP", 6))

/* 检测MFPTP协议的版本号是否正确 */
#define CHECK_VERSION(parser)	(parser->header.major_version == MFPTP_MAJOR_VERSION &&	\
				parser->header.minor_version == MFPTP_MINOR_VERSION )

/* 检测MFPTP协议压缩加密设置是否正确 */
#define CHECK_CONFIG(parser)	(parser->header.compression <= GZIP_COMPRESSION &&	\
				parser->header.compression >= NO_COMPRESSION &&		\
				parser->header.encryption >= NO_ENCRYPTION &&		\
				parser->header.encryption <= AES_ENCRYPTION)

/* 检测MFPTP协议F_SIZE所占的字节数是否是在1-4范围内 */
#define CHECK_F_SIZE(parser)	(parser->header.size_f_size > 0 && \
				 parser->header.size_f_size <= 4)

/* 检测MFPTP协议socket type是否正确*/
#define	CHECK_SOCKTYPE(parser)	(parser->header.socket_type <= HEARTBEAT_METHOD	&&	\
				 parser->header.socket_type >= PAIR_METHOD)

/* 检测MFPTP协议包数是否是在范围之内 */
#define CHECK_PACKAGES(parser)	(parser->header.packages > 0 && \
				 parser->header.packages <= MFPTP_MAX_PACKAGES)

/* 检测MFPTP协议帧携带的数据大小是否正确 */
#define CHECK_DATASIZE(parser) (*parser->ms.dsize-parser->ms.dosize >= parser->header.f_size)


/* MFPTP协议压缩加密值 */
enum mfptp_config {
	NO_COMPRESSION		= 0x00 << 4,
	ZIP_COMPRESSION		= 0x01 << 4,
	GZIP_COMPRESSION	= 0x02 << 4,
	NO_ENCRYPTION		= 0x00,
	IDEA_ENCRYPTION		= 0x01,
	AES_ENCRYPTION		= 0x02
};

/* MFPTP协议的socket的type值 */
enum mfptp_socket_type {
	PAIR_METHOD = 0x00,
	PUB_METHOD,
	SUB_METHOD,
	REQ_METHOD,
	REP_METHOD,
	DEALER_METHOD,
	ROUTER_METHOD,
	PULL_METHOD,
	PUSH_METHOD,
	HEARTBEAT_METHOD,
	INVALID_METHOD
};

/* MFPTP协议错误码 */
enum mfptp_error {
	MFPTP_OK = 0x00,		/* MFPTP打包解析的时候没发生错误，继续下一步 */
	MFPTP_HEAD_INVAILD,		/* MFPTP协议解析前6个字节“#MFPTP”出错 */
	MFPTP_VERSION_INVAILD,		/* MFPTP协议版本无效 */
	MFPTP_CONFIG_INVAILD,		/* MFPTP压缩解密格式设置错误 */
	MFPTP_SOCKTYPE_INVAILD,		/* MFPTP协议socket_type设置错误 */
	MFPTP_PACKAGES_INVAILD,		/* MFPTP协议包数不合法[小于零或大于允许携带的最大包数] */
	MFPTP_DATA_TOOFEW,		/* MFPTP协议帧携带的数据太少[数据未接收完毕] */
	MFPTP_DATASIZE_INVAILD		/* MFPTP协议帧携带的数据长度无效[携带的数据小于零或者大于允许帧携带最大数] */

};

/* MFPTP协议帧的相关信息 */
struct mfptp_frame_info {
	int frame_size;			/* 帧的大小 */
	int frame_offset;		/* 帧的偏移 */
};

/* MFPTP协议包的相关信息 */
struct mfptp_package_info {
	int			frames;				/* 此包帧的总数 */
	struct mfptp_frame_info frame[MFPTP_MAX_FRAMES];	/* 每帧的相关信息 */
};


/* MFPTP协议消息的相关信息 */
struct mfptp_bodyer_info {
	int			  dsize;			/* 消息的总大小 */
	int			  packages;			/* 包的总数 */
	struct mfptp_package_info package[MFPTP_MAX_PACKAGES];	/* 每个包的相关信息 */
};

/* MFPTP协议的包头相关信息 */
struct mfptp_header_info {
	int		packages;			/* 包的数量 */
	unsigned int	f_size;				/* F_size字段的值[帧所携带数据大小] */
	unsigned char	not_end;			/* 当前帧是否是最后一帧 */
	unsigned char	size_f_size;			/* F_size字段所占字节数 */
	unsigned char	encryption;			/* 加密格式 */
	unsigned char	compression;			/* 压缩格式 */
	unsigned char	socket_type;			/* socket的类型 */
	unsigned char	major_version;			/* 主版本号 */
	unsigned char	minor_version;			/* 副版本号*/
};

     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __MFPTP_UTILS_H__ */
