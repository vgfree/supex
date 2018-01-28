#pragma once

#include <stdint.h>

/*
 *    fd_head
 *    ---------------         ---------         ---------
 *    |CLIENT_ROUTER| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------------         ---------         ---------
 *    ---------------         ---------         ---------
 *    |STATUS_ROUTER| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------------         ---------         ---------
 *    ---------------         ---------         ---------
 *    |STREAM_ROUTER| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------------         ---------         ---------
 *    ---------------         ---------         ---------
 *    |MANAGE_ROUTER| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------------         ---------         ---------
 */
enum router_type
{
	CLIENT_ROUTER = 0,
	STATUS_ROUTER,
	STREAM_ROUTER,
	MANAGE_ROUTER,
	MAX_ROUTER_KIND
};

/*
 *   构建的数据结构需要做到两点快速访问。
 *   1.根据fd能找到对应的相关状态及描述。
 *   2.根据相关描述快速定位到相关fd。
 */
struct fd_node
{
	int             fd;
	uint8_t         status;	// Descriptor fd connect status.
	struct fd_node  *next;
};

struct fd_head
{
	enum router_type        type;
	uint32_t                size;
	struct fd_node          *next;
};

struct fd_list
{
	struct fd_head head[MAX_ROUTER_KIND];
};

int fdman_list_init(void);

int fdman_list_free(void);

int fdman_list_del(const enum router_type type, const int fd);

int fdman_list_add(const enum router_type type, const struct fd_node *node);

int fdman_list_top(const enum router_type type, struct fd_node *node);

struct fd_info
{
	uint32_t                host;
	uint16_t                port;
	uint8_t                 status;	// 0 未使用， 1 连接正常
	enum router_type        type;
	char                    uuid[37];
};

struct fd_slot
{
	struct fd_info  *info;
	int             max;			/*记录最大使用值，减少遍历*/
	int             cap;			/*记录最大容量*/
};

int fdman_slot_init(void);

int fdman_slot_free(void);

int fdman_slot_set(const int fd, const struct fd_info *des);

int fdman_slot_del(const int fd);

int fdman_slot_get(const int fd, struct fd_info *des);

