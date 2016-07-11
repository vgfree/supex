#ifndef _FD_MANAGER_H_
#define _FD_MANAGER_H_

#include "router.h"

#define FD_CAPACITY             1024 * 100
#define FD_MAX_CLASSIFICATION   5
#define SUCCESS                 0
#define FAILED                  -1

/*    fd_head
 *    ---------         ---------         ---------
 *    | CLIENT| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------         ---------         ---------
 *    |GATEWAY| ----->  |fd_node|  -----> |fd_node| ...
 *    ---------         ---------         ---------
 *      ...               ...               ...
 * */

/*
 *   构建的数据结构需要做到两点快速访问。
 *   1, 根据fd, 能找到对应fd 的相关状态以及fd所相关的一切描述。
 *   2, 根据， 相关描述快速定位到相关fd,
 */

struct fd_node;

struct fd_node
{
	int             fd;
	uint8_t         status;	// Descriptor fd connect status.
	struct fd_node  *next;
};

struct fd_head
{
	enum router_object      obj;
	uint32_t                size;
	struct fd_node          *next;
};

struct fd_list
{
	struct fd_head head[FD_MAX_CLASSIFICATION];
};

int list_init();

int list_destroy();

int list_remove(const enum router_object obj, const int fd);

int list_push_back(const enum router_object     obj,
	const struct fd_node                    *node);

int list_front(const enum router_object obj,
	struct fd_node                  *node);

struct fd_descriptor
{
	uint32_t                ip;
	uint16_t                port;
	uint8_t                 status;	// 0 未使用， 1 连接正常， 2 已经closed掉。
	enum router_object      obj;
};

struct fd_array
{
	struct fd_descriptor    *dsp_array;
	int                     max_fd;
	int                     cap;
};

int array_init();

int array_destroy();

int array_fill_fd(const int fd, const struct fd_descriptor *des);

int array_remove_fd(const int fd);

int array_at_fd(const int fd, struct fd_descriptor *des);

uint32_t statistic_object(const enum router_object obj);

uint32_t poll_client_fd(int *arr[], uint32_t *size);
#endif	/* ifndef _FD_MANAGER_H_ */

