#include <stdio.h>
#include <string.h>
#include "../mfptp_protocol/mfptp_package.h"
#include "../mfptp_protocol/mfptp_parse.h"
#include "../comm_structure.h"

#define	MAXDATASIZE	4194304

void packager(struct comm_cache *cache);

void parser(struct comm_cache *pack_cache, struct comm_cache *parse_cache);

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser);

int main()
{
	int pckbuff_size = 0;
	struct comm_cache pack_cache = {};
	struct comm_cache parse_cache = {};
	commcache_init(&pack_cache);
	commcache_init(&parse_cache);

	packager(&pack_cache);
	parser(&pack_cache, &parse_cache);
}

void packager(struct comm_cache *cache)
{
	char buff[MAXDATASIZE] = "you never know what's gonna happen to you";
	int dsize = strlen(buff);
	int packages = 1;
	int pckidx = 0;
	int frmidx = 0;
	int index = 0;
	int size = 0;

	struct mfptp_packager packager = {};
	struct comm_message message = {};
	int frames_of_package[10] = {8};
	int frame_size[64] = { 
		strlen("you "),
		strlen("never "),
		strlen("know "),
		strlen("what's "),
		strlen("gonna "),
		strlen("happen "),
		strlen("to "),
		strlen("you")
	};
	int frame_offset[46] = { 0,
		strlen("you "),
		strlen("you never "),
		strlen("you never know "),
		strlen("you never know what's "),
		strlen("you never know what's gonna "),
		strlen("you never know what's gonna happen "),
		strlen("you never know what's gonna happen to ")
	};

	mfptp_package_init(&packager, &cache->buffer, &cache->size);
	//message.fd = fd;
	message.content = buff;
	message.config = IDEA_ENCRYPTION | GZIP_COMPRESSION;
	message.socket_type = REQ_METHOD;
	message.package.dsize = strlen(buff);
	message.package.frames = 8;
	message.package.packages = 1;

	for (pckidx = 0, index = 0 ; pckidx < message.package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, index++) {
			message.package.frame_offset[index] = frame_offset[index];
			message.package.frame_size[index] = frame_size[index];
		}
	} 
	memcpy(message.package.frames_of_package, frames_of_package, (sizeof(int))*message.package.packages);
	size = mfptp_check_memory(cache->capacity - cache->size, message.package.frames, message.package.dsize);
	if (size > 0) {
		/* 检测到内存不够 则增加内存*/
		if (commcache_expend(cache, size) == false ) {
			printf("expend commcache failed\n");
			return ;
		}
		log("expend commcache successed\n");
	}
	mfptp_fill_package(&packager, message.package.frame_offset, message.package.frame_size, message.package.frames_of_package, message.package.packages);
	size = mfptp_package(&packager, message.content, message.config, message.socket_type);
	if (size > 0 && packager.ms.error == MFPTP_OK) {
		printf("packager successed\n");
		cache->end += size;
	} else {
		printf("packager failed\n");
		packager.ms.error = MFPTP_OK;
	}
	return ;
}

void parser(struct comm_cache *pack_cache, struct comm_cache *parse_cache)
{
	char buff[1024] = {};
	int len = pack_cache->size/2;
	int pckidx = 0;
	int frmidx = 0;
	int index = 0;
	int size = 0;

	struct mfptp_parser parser = {};
	struct comm_message message = {};
	message.content = malloc(MAXDATASIZE);
	memset(message.content, 0, MAXDATASIZE);

	mfptp_parse_init(&parser, &parse_cache->buffer, &parse_cache->size);
	commcache_append(parse_cache, pack_cache->buffer, len);
	size = mfptp_parse(&parser);
	if ((size > 0) && parser.ms.error == MFPTP_OK && parser.ms.step == MFPTP_PARSE_OVER) {	/* 成功解析了一个连续的包 */
		memcpy(message.content, &parser.ms.cache.buffer[parser.ms.cache.start], parser.bodyer.dsize);
		_fill_message_package(&message, &parser);
		printf("parse success\n");
	} else if (parser.ms.error == MFPTP_DATA_TOOFEW) {
		printf("data too few\n");
		parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
		commcache_append(parse_cache, &pack_cache->buffer[len], pack_cache->size - len);
		size = mfptp_parse(&parser);
		memcpy(message.content, &parser.ms.cache.buffer[parser.ms.cache.start], parser.bodyer.dsize);
		_fill_message_package(&message, &parser);
		if (size > 0 && parser.ms.error == MFPTP_OK && parser.ms.step == MFPTP_PARSE_OVER) {
			for (pckidx = 0, index = 0; pckidx < message.package.packages;  pckidx ++) {
				size = 0;
				for (frmidx = 0; frmidx < message.package.frames_of_package[pckidx]; frmidx++, index++) {
					memcpy(&buff[size], &message.content[message.package.frame_offset[index]], message.package.frame_size[index]);
					size += message.package.frame_size[index];
				}
				log("message body: %s\n", buff);
			}
		} else {
			printf("send parse failed\n");
		}
		return ;
	} else if (parser.ms.error != MFPTP_DATA_TOOFEW) {
		/* 解析出错 抛弃已解析的错误数据 继续解析后面的数据 */
		parse_cache->start += size;
		parse_cache->size -= size;
		commcache_clean(parse_cache);
		parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
		printf("parse failed\n");
	}
	return ;
}

/* 填充message结构体 */ 
static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser) 
{
	assert(message && parser);
	int k = 0;
	int pckidx = 0;		/* 包的索引 */
	int frmidx = 0;		/* 帧的索引 */
	int frames = 0;		/* 总帧数 */
	const struct mfptp_bodyer_info* bodyer = &parser->bodyer;
	const struct mfptp_header_info* header = &parser->header;
	for (pckidx = 0; pckidx < bodyer->packages; pckidx++) {
		for (frmidx = 0; frmidx < bodyer->package[pckidx].frames; frmidx++,k++) {
			message->package.frame_size[k] = bodyer->package[pckidx].frame[frmidx].frame_size;
			message->package.frame_offset[k] = bodyer->package[pckidx].frame[frmidx].frame_offset - parser->ms.cache.start;
		}
		message->package.frames_of_package[pckidx] = bodyer->package[pckidx].frames;
		frames += bodyer->package[pckidx].frames;
	}
	message->package.packages = bodyer->packages;
	message->package.frames = frames;
	message->package.dsize = bodyer->dsize;

	message->config = header->compression | header->encryption;
	message->socket_type = header->socket_type;
}
