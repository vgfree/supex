#include <stdio.h>
#include <string.h>
#include "../mfptp_protocol/mfptp_package.h"
#include "../mfptp_protocol/mfptp_parse.h"
#include "../comm_structure.h"

#define MAXDATASIZE 0x22222222

bool packager(struct comm_cache *cache);

void parser(struct comm_cache *pack_cache, struct comm_cache *parse_cache);

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser);

static bool _check_packageinfo(const struct comm_message *message);

int main()
{
	int                     pckbuff_size = 0;
	struct comm_cache       pack_cache = {};
	struct comm_cache       parse_cache = {};

	commcache_init(&pack_cache);
	commcache_append(&pack_cache, "#ihi", 4);
	commcache_init(&parse_cache);

	if (packager(&pack_cache)) {
		parser(&pack_cache, &parse_cache);
	}
}

bool packager(struct comm_cache *cache)
{
//	char    buff[MAXDATASIZE] = "you never know what's gonna happen to you";
	char*	buff = malloc(MAXDATASIZE);
	memcpy(buff, "you never know what's gonna happen to you", strlen("you never know what's gonna happen to you"));
	int     dsize = strlen(buff);
	int     packages = 1;
	int     pckidx = 0;
	int     frmidx = 0;
	int     index = 0;
	int     size = 0;

	struct mfptp_packager   packager = {};
	struct comm_message     message = {};
	int                     frames_of_package[10] = {8};
	int                     frame_size[64] = {
		strlen("you "),
		strlen("never "),
		strlen("know "),
		strlen("what's "),
		strlen("gonna "),
		strlen("happen "),
		strlen("to "),
		strlen("you")
	};
	int                     frame_offset[46] = { 0,
						     strlen("you "),
						     strlen("you never "),
						     strlen("you never know "),
						     strlen("you never know what's "),
						     strlen("you never know what's gonna "),
						     strlen("you never know what's gonna happen "),
						     strlen("you never know what's gonna happen to ") };

	mfptp_package_init(&packager, &cache->buffer, &cache->size);
	// message.fd = fd;
	message.content = buff;
//	message.config = IDEA_ENCRYPTION | GZIP_COMPRESSION;
	message.config = NO_ENCRYPTION | NO_COMPRESSION;
	message.socket_type = REQ_METHOD;
	message.package.dsize = strlen(buff);
	//message.package.dsize =	0x1110245;
	message.package.frames = 9;
	message.package.packages = 1;

	for (pckidx = 0, index = 0; pckidx < message.package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, index++) {
			message.package.frame_offset[index] = frame_offset[index];
			message.package.frame_size[index] = frame_size[index];
		}
	}

	message.package.frame_offset[7] = message.package.frame_offset[7] + 1;
	memcpy(message.package.frames_of_package, frames_of_package, (sizeof(int)) * message.package.packages);
	size = mfptp_check_memory(cache->capacity - cache->size, message.package.frames, message.package.dsize);

	if (size > 0) {
		/* 检测到内存不够 则增加内存*/
		if (commcache_expend(cache, size) == false) {
			printf("expend commcache failed\n");
			return false;
		}

		log("expend commcache successed\n");
	}
	message.package.frame_size[7] = 12;
	//message.package.frame_size[7] = 0x1110245 - (strlen(buff)-3);
	//printf("frame_size[7] 0x%x strlen(buff):%d\n", message.package.frame_size[7], strlen(buff));

	if (_check_packageinfo(&message)) {
		mfptp_fill_package(&packager, message.package.frame_offset, message.package.frame_size, message.package.frames_of_package, message.package.packages);
		size = mfptp_package(&packager, message.content, message.socket_type);

		if ((size > 0) && (packager.ms.error == MFPTP_OK)) {
			printf("packager successed\n");
			cache->end += size;
			free(buff);
			return true;
		} else {
			printf("packager failed\n");
			packager.ms.error = MFPTP_OK;
			return false;
		}
	}
	log("check packageinfo failed\n");
	return false;
}

void parser(struct comm_cache *pack_cache, struct comm_cache *parse_cache)
{
	//char    buff[MAXDATASIZE] = {};
	char*	buff = malloc(MAXDATASIZE);
	int     len = pack_cache->size / 2;
	int     pckidx = 0;
	int     frmidx = 0;
	int     index = 0;
	int     size = 0;

	struct mfptp_parser     parser = {};
	struct comm_message     message = {};

	message.content = malloc(MAXDATASIZE);
	memset(message.content, 0, MAXDATASIZE);

	mfptp_parse_init(&parser, &parse_cache->buffer, &parse_cache->size);
	commcache_append(parse_cache, pack_cache->buffer, len);
	while (1) {
		size = mfptp_parse(&parser);
		if ((size > 0) && (parser.ms.error == MFPTP_OK) && (parser.ms.step == MFPTP_PARSE_OVER)) {	/* 成功解析了一个连续的包 */
			_fill_message_package(&message, &parser);
			for (pckidx = 0, index = 0; pckidx < message.package.packages; pckidx++) {
#if 0
				size = 0;
				for (frmidx = 0; frmidx < message.package.frames_of_package[pckidx]; frmidx++, index++) {
					memcpy(&buff[size], &message.content[message.package.frame_offset[index]], message.package.frame_size[index]);
					size += message.package.frame_size[index];
					log("message frame body: %*.s\n", size, buff);
				}
#endif

				log("message body: %s datasize:0x%x\n", message.content, message.package.dsize);
			}
			free(buff);
			free(message.content);
			printf("parse success\n");
			return ;
		} else if (parser.ms.error == MFPTP_DATA_TOOFEW) {
			printf("data too few\n");
			parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
			commcache_append(parse_cache, &pack_cache->buffer[len], pack_cache->size - len);
		} else if (parser.ms.error != MFPTP_DATA_TOOFEW) {
			/* 解析出错 抛弃已解析的错误数据 继续解析后面的数据 */
			parse_cache->start += size;
			parse_cache->size -= size;
			commcache_clean(parse_cache);
			parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
			printf("parse failed\n");
		}
	}
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
		log("wrong packages in comm_message structure, packages:%d", message->package.packages);
		return false;
	}
	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		if (message->package.frames_of_package[pckidx] < 1 || message->package.frames_of_package[pckidx] > message->package.frames) { 
			log("wrong sum of frames in frames_of_pack of comm_message structure, frames:%d, index:%d\n", message->package.frames_of_package[pckidx], pckidx);
			return false;
		}
		for (frmidx = 0 ; frmidx < message->package.frames_of_package[pckidx]; frmidx++, index++) {
			if (unlikely(dsize > message->package.dsize || message->package.frame_size[index] > message->package.dsize)) {
				log("wrong frame_size in comm_message structure, frame_size:%d index:%d\n", message->package.frame_size[index], index);
				return false;
			}
			if (unlikely(message->package.frame_offset[index] != dsize)) {
				log("wrong frame_offset in comm_package, frame_offset:%d index:%d\n", message->package.frame_offset[index], index);
				return false;
			}
			dsize += message->package.frame_size[index];
			frames++;
		}
	}

	if (unlikely(frames != message->package.frames)) {
		log("wrong sum of frames in comm_message structure, frames:%d\n",message->package.frames);
		return false;
	}
	if (unlikely(dsize != message->package.dsize)) {
		log("wrong sum of datasize in comm_message structure, datasize:%d\n", message->package.dsize);
		return false;
	}
	return true;
}
