#include <stdio.h>
#include <string.h>
#include "../mfptp_protocol/mfptp_package.h"
#include "../mfptp_protocol/mfptp_parse.h"
#include "../comm_structure.h"
#include "../comm_disposedata.h"


bool packager(char **pckbuff, int *pckbuff_size);

void parser(const char *pckbuff, int pckbuff_size);

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser);

static bool _check_packageinfo(const struct comm_message *message);

int main()
{
	char	buff[1024] = {};		/* 保存打包成功的数据缓冲区 */
	char*	pckbuff = buff;
	int	pckbuff_size = 1024;		/* 保存打包成功数据缓冲区大小 */

	if (packager(&pckbuff, &pckbuff_size)) {
		parser(pckbuff, pckbuff_size);
	}

	if (pckbuff != buff) {
		free(pckbuff);
	}

	/*****************************************************/
#include <assert.h>
#define DO_FILE "data"
	int     fd = open(DO_FILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		fprintf(stderr, "Open %s\n", DO_FILE);
		return -1;
	}
	int     bytes = write(fd, pckbuff, pckbuff_size);
	assert(bytes == pckbuff_size);
	close(fd);
	/*****************************************************/
	return 0;
}

/* @pckbuff_size: 值-结果参数，传进来时代表的是保存打包成功数据的缓冲区大小 出去时代表的是缓冲去已有数据的大小 */
bool packager(char **pckbuff, int *pckbuff_size)
{
	char    data[1024] = "you never know what's gonna happen to you";	/* 待打包的数据 */
	int     dsize = strlen(data);						/* 待打包数据的总大小 */
	int     packages = 1;							/* 待打包的总包数 */
	int	frames = 8;							/* 待打包的总帧数 */
	int	size = 0;							/* 用来检测缓冲区大小是否足够 */
	int	frames_of_package[10] = {8};					/* 每个单包中的总帧数 */
	int	frame_size[64] = {						/* 每帧的数据大小 */
			strlen("you "),
			strlen("never "),
			strlen("know "),
			strlen("what's "),
			strlen("gonna "),
			strlen("happen "),
			strlen("to "),
			strlen("you")
		};
	int	frame_offset[46] = { 0,		/* 每帧的数据偏移 */
			strlen("you "),
			strlen("you never "),
			strlen("you never know "),
			strlen("you never know what's "),
			strlen("you never know what's gonna "),
			strlen("you never know what's gonna happen "),
			strlen("you never know what's gonna happen to ") 
		};
	struct mfptp_packager   packager = {};	/* 打包器结构体 */

	mfptp_package_init(&packager, pckbuff, pckbuff_size);
	size = mfptp_check_memory(*pckbuff_size, frames, dsize);
	if (size > 0) {
		/* 检测到内存不够 则增加内存 size为欠缺的内存的大小 */
		*pckbuff = malloc(*pckbuff_size + size);
		if (*pckbuff != NULL) {
			*pckbuff_size += size;
			memset(*pckbuff, 0, *pckbuff_size + size);
		} else {
			return false;
		}
	}
	*pckbuff_size = 0;	/* 从此刻起代表的就是缓冲区已有数据的大小 */
	mfptp_fill_package(&packager, frame_offset, frame_size, frames_of_package, packages);
	size = mfptp_package(&packager, data, NO_ENCRYPTION|NO_COMPRESSION, REQ_METHOD);
	if ((size > 0) && (packager.ms.error == MFPTP_OK)) {
		printf("packager successed\n");
		return true;
	} else {
		printf("packager failed\n");
		packager.ms.error = MFPTP_OK;
		return false;
	}
}

void parser(const char *pckbuff, int pckbuff_size)
{
	int	size = 0;
	char	buff[1024] = {};		/* 保存解析之后的数据 */
	struct comm_cache	cache = {};
	struct mfptp_parser	parser = {};
	struct comm_message	message = {};

	message.content = buff;
	commcache_init(&cache);
	commcache_append(&cache, pckbuff, pckbuff_size);
	mfptp_parse_init(&parser, &cache.buffer, &cache.size);
	while (1) {
		size = mfptp_parse(&parser);
		if (unlikely(size == 0)) {
			/* 数据未接收完毕 */
			continue ;
		}
		if (unlikely(parser.ms.error != MFPTP_OK)) {
			printf("parser failed\n");
			return ;
		}
		_fill_message_package(&message, &parser);
		cache.size -= size;
		cache.start += size;
		commcache_clean(&cache);
		printf("parse success\n");
		loger(" message body: %.*s \n datasize:%d\n", message.package.dsize, message.content, message.package.dsize);
		break ;
	}
	commcache_free(&cache);
	return ;
}

/* 填充message结构体 */
static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser)
{
	assert(message && parser);
	int				index = 0;
	int                             pckidx = 0;	/* 包的索引 */
	int                             frmidx = 0;	/* 帧的索引 */
	int                             frames = 0;	/* 总帧数 */
	const char                      *data = *parser->ms.data;	/* 待解析的数据缓冲区 */
	const struct mfptp_bodyer_info  *bodyer = &parser->bodyer;
	const struct mfptp_header_info  *header = &parser->header;

	for (pckidx = 0, index = 0; pckidx < bodyer->packages; pckidx++) {
		for (frmidx = 0; frmidx < bodyer->package[pckidx].frames; frmidx++, index++) {
			message->package.frame_offset[index] += message->package.dsize;
			message->package.frame_size[index] = bodyer->package[pckidx].frame[frmidx].frame_size;
			memcpy(&message->content[message->package.dsize], &data[bodyer->package[pckidx].frame[frmidx].frame_offset], bodyer->package[pckidx].frame[frmidx].frame_size);
			message->package.dsize += bodyer->package[pckidx].frame[frmidx].frame_size;
		}
		message->package.frames_of_package[pckidx] = bodyer->package[pckidx].frames;
		frames += bodyer->package[pckidx].frames;
	}
	message->package.packages = bodyer->packages;
	message->package.frames = frames;

	message->config = header->compression | header->encryption;
	printf("compression:%d encryption:%d\n", header->compression, header->encryption);
	message->socket_type = header->socket_type;
}

/* 检测包的信息是否设置正确 */
static bool _check_packageinfo(const struct comm_message *message)
{
	assert(message);
	int     index = 0;
	int     dsize = 0;	/* 数据的总大小 */
	int     pckidx = 0;	/* 包的索引 */
	int     frmidx = 0;	/* 帧的索引 */
	int	frames = 0;

	if (unlikely(message->package.packages < 1)) {
		loger("wrong packages in comm_message structure, packages:%d", message->package.packages);
		return false;
	}
	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		if (message->package.frames_of_package[pckidx] < 1 || message->package.frames_of_package[pckidx] > message->package.frames) { 
			loger("wrong sum of frames in frames_of_pack of comm_message structure, frames:%d, index:%d\n", message->package.frames_of_package[pckidx], pckidx);
			return false;
		}
		for (frmidx = 0 ; frmidx < message->package.frames_of_package[pckidx]; frmidx++, index++) {
			if (unlikely(dsize > message->package.dsize || message->package.frame_size[index] > message->package.dsize)) {
				loger("wrong frame_size in comm_message structure, frame_size:%d index:%d\n", message->package.frame_size[index], index);
				return false;
			}
			if (unlikely(message->package.frame_offset[index] != dsize)) {
				loger("wrong frame_offset in comm_package, frame_offset:%d index:%d\n", message->package.frame_offset[index], index);
				return false;
			}
			dsize += message->package.frame_size[index];
			frames++;
		}
	}

	if (unlikely(frames != message->package.frames)) {
		loger("wrong sum of frames in comm_message structure, frames:%d\n",message->package.frames);
		return false;
	}
	if (unlikely(dsize != message->package.dsize)) {
		loger("wrong sum of datasize in comm_message structure, datasize:%d\n", message->package.dsize);
		return false;
	}
	return true;
}
