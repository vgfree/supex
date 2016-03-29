#include <stdio.h>
#include <string.h>
#include "tcp_api/connection_pool/connection_pool.h"
#include "tcp_api/tcp_io.h"
#include "pool_api/pool.h"
#include "evcoro_scheduler.h"

#define POOLSIZE        5
#define STACK_SIZE      1024
#define Cache           (client->pool->conn->cache)
#define Cache_Buff      (client->pool->conn->cache.buff)
#define Cache_Start     (client->pool->conn->cache.start)
#define Cache_End       (client->pool->conn->cache.end)

struct client
{
	char            *host;
	char            *server;
	bool            end;	// 文件是否已经读到结尾
	int             fd;	// 读取的文件描述符
	struct connpool *pool;
};

void send_data(struct evcoro_scheduler *scheduler, void *usr);

void recv_data(struct evcoro_scheduler *scheduler, void *usr);

void evcoro_idle(struct evcoro_scheduler *scheduler, void *usr);

void tcp_idle(struct tcp_socket *tcp, void *usr);

int tcp_parse(struct tcp_socket *tcp, void *usr);

static inline struct client     *_new_client(char *host, char *server);

static void _free_client(void *usr);

int main(int argc, char *argv[])
{
	if (unlikely(argc < 3)) {
		x_printf(D, "usage: %s: <host/ip> <server/port>", argv[1]);
		return 0;
	}

	int volatile                            *counter = NULL;
	struct client volatile                  *client = NULL;
	struct evcoro_scheduler volatile        *scheduler = NULL;

	TRY
	{
		int ret = -1;
		client = _new_client(argv[1], argv[2]);

		New(counter);
		AssertError(counter, ENOMEM);
		*counter = 0;
		// 创建一个协程句柄
		scheduler = evcoro_create(-1);
		AssertError(scheduler, ENOMEM);

		// 创建一个连接池
		ret = connpool_create(client->host, client->server, POOLSIZE, true);
		RAISE_SYS_ERROR(ret);

		// 启动协程
		scheduler->user = (void *)counter;
		evcoro_loop((struct evcoro_scheduler *)scheduler, evcoro_idle, (void *)client);
	}
	FINALLY
	{
		_free_client((void *)client);
		Free(scheduler->user);
		evcoro_destroy((struct evcoro_scheduler *)scheduler, NULL);
	}
	END;

	return 0;
}

// 添加的协程任务，switch的时候会保存当前的状态，被切换回来之后接着上次执行的位置继续运行
void send_data(struct evcoro_scheduler *scheduler, void *usr)
{
	struct client volatile *client = (struct client volatile *)usr;

	TRY
	{
		int     flag = EVCORO_WRITE;
		int     ret = -1, bytes = -1;
		AssertError(client, EINVAL);

		evcoro_cleanup_push(scheduler, _free_client, (void *)client);	// 压入一个清理函数

		// 从连接池中取出一个tcp发送数据
		ret = connpool_pull(client->host, client->server, client->pool);
		RAISE_SYS_ERROR(ret);
		bytes = read(client->fd, &Cache_Buff[Cache_End], 256);
		RAISE_SYS_ERROR(bytes);

		if (likely(bytes > 0)) {
			Cache_End += bytes;
		} else {
			// 文件内容已读完
			strncpy(&Cache_Buff[Cache_End], "EOF", strlen("EOF"));
			Cache_End += strlen("EOF");
			client->end = true;
		}

		client->pool->conn->usr = scheduler;
		x_printf(D, "start to send data fd:%d pool:%x", client->pool->conn->fd, client->pool->pool);
		ret = tcp_send(client->pool->conn, tcp_idle, (void *)&flag);

		if (likely(ret > 0)) {
			x_printf(D, "%s:%s:%d send data successed", client->host, client->server, client->pool->conn->fd);
			connpool_push(client->pool);	// tcp用完之后放回池中
			cache_cutmem(&Cache);		// 清理cache中的无效数据[已使用过无需再使用]
			evcoro_push(scheduler, recv_data, (void *)client, STACK_SIZE);
			evcoro_fastswitch(scheduler);
		}
	}
	CATCH
	{
		x_perror("send data to peer failed:%s", x_strerror(errno));
	}
	FINALLY
	{
		// x_printf(D, "send data finished %d", client->pool->conn->fd);
		evcoro_cleanup_pop(scheduler, false);	// 弹出清理函数但并不执行清理函数
	}
	END;
}

void recv_data(struct evcoro_scheduler *scheduler, void *usr)
{
	struct client volatile *client = (struct client volatile *)usr;

	TRY
	{
		int     ret = -1;
		int     flag = EVCORO_READ;
		AssertError(client, EINVAL);

		evcoro_cleanup_push(scheduler, _free_client, (void *)client);

		// 从连接池中取出一个tcp发送数据
		ret = connpool_pull(client->host, client->server, client->pool);
		RAISE_SYS_ERROR(ret);

		client->pool->conn->usr = scheduler;
		x_printf(D, "start to recvied data fd:%d, pool:%x", client->pool->conn->fd, client->pool->pool);
		ret = tcp_recv(client->pool->conn, tcp_parse, tcp_idle, (void *)&flag);
		RAISE_SYS_ERROR(ret);

		if (likely(ret > 0)) {
			x_printf(D, "%s:%s:%d recevied data: %.*s", client->host, client->server, client->pool->conn->fd, Cache_End - Cache_Start, &Cache_Buff[Cache_Start]);

			if (unlikely(client->end)) {
				x_printf(D, "recv_data the end of file");
				evcoro_cleanup_pop(scheduler, true);				// 弹出清理任务并执行清理任务
			} else {
				cache_clean(&Cache);						// 清空cache
				connpool_push(client->pool);					// tcp用完之后放回池中
				evcoro_push(scheduler, send_data, (void *)client, STACK_SIZE);	// 添加一个发送数据的任务
				evcoro_fastswitch(scheduler);
				evcoro_cleanup_pop(scheduler, false);
			}
		} else {
			x_printf(D, "server already closed %s:%s:%d", client->host, client->server, client->pool->conn->fd);
			evcoro_cleanup_pop(scheduler, true);	// 弹出清理任务并执行清理任务
		}
	}
	CATCH
	{
		x_perror("send data to peer failed:%s", x_strerror(errno));
		evcoro_cleanup_pop(scheduler, true);	// 弹出清理任务并执行清理任务
	}
	END;
}

// 协程中idle函数 无协程任务时一直执行此函数 有协程任务时则所有任务循环一轮执行一次  无法保存状态，一次性执行完毕
void evcoro_idle(struct evcoro_scheduler *scheduler, void *usr)
{
	AssertError(usr, EINVAL);

	struct client   *client = NULL;
	struct client   *arg = (struct client *)usr;

	if ((*(int *)scheduler->user) == POOLSIZE) {
		evcoro_stop(scheduler);
		// sleep(1);
		return;
	}

	client = _new_client(arg->host, arg->server);

	if (unlikely(!client)) {
		return;
	}

	*((int *)scheduler->user) += 1;
	evcoro_push(scheduler, send_data, (void *)client, STACK_SIZE);
}

// tcp中发送接收数据时的idle函数，无数据可读或者对端没准备好接收数据时执行此函数
void tcp_idle(struct tcp_socket *tcp, void *usr)
{
	union evcoro_event event = {};

	evcoro_io_init(&event, tcp->fd, 2000);
	// 将此任务放入到协程挂起链表中直到2秒钟之后或此描述符可读写时才重新被放入到协程的工作链表中
	evcoro_idleswitch((struct evcoro_scheduler *)tcp->usr, &event, *((int *)usr));
}

int tcp_parse(struct tcp_socket *tcp, void *usr)
{
	x_printf(D, "don't need to exam, everything is fine to me fd:%d", tcp->fd);
	return 1;
}

static inline struct client *_new_client(char *host, char *server)
{
	int             ret = -1;
	char            filename[256] = "Makefile";
	struct client   *client = NULL;

	New(client);
	AssertError(client, ENOMEM);
	memset(client, 0, sizeof(*client));

	New(client->pool);
	AssertError(client->pool, ENOMEM);

	client->host = x_strdup(host);
	AssertError(client->host, ENOMEM);
	client->server = x_strdup(server);
	AssertError(client->server, ENOMEM);

	client->fd = open(filename, O_NOATIME | O_RDONLY);
	RAISE_SYS_ERROR(client->fd);

	return client;
}

static void _free_client(void *usr)
{
	struct client *client = (struct client *)usr;

	AssertError(client, EINVAL);

	if (likely(client->fd > 0)) {
		close(client->fd);
	}

	if (likely(client->pool)) {
		connpool_disconn(client->pool);
	}

	Free(client->pool);
	Free(client->host);
	Free(client->server);
	Free(client);
}

