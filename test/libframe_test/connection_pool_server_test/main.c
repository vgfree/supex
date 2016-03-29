#include <stdio.h>

#include "libmini.h"
#include "tcp_api/tcp_io.h"
#include "tcp_api/connection_pool/connection_pool.h"

#define         MAXTHREADS 2000

struct threadinfo
{
	pthread_t       threadid[MAXTHREADS];
	unsigned int    counter;
};

static int SendData(struct tcp_socket *tcp);

static void _deal_signal(int signo);

void *ReceiveData(void *usr);

void Accept(struct tcp_socket *tcp);

int main(int argc, char *argv[])
{
	struct tcp_socket volatile *tcp = NULL;

	TRY
	{
		SignalIntr(SIGINT, SignalNormalCB);
		tcp = tcp_listen(argv[1], argv[2]);
		AssertError(tcp, ENOMEM);
		Accept((struct tcp_socket *)tcp);
	}
	CATCH
	{
		x_perror("error:%s", x_strerror(errno));
	}
	FINALLY
	{
		tcp_destroy((struct tcp_socket *)tcp);
	}
	END;
	return 0;
}

void Accept(struct tcp_socket *tcp)
{
	struct tcp_socket volatile      *client_tcp = NULL;
	struct threadinfo volatile      threadinfo = {};

	TRY
	{
		while (1) {
			int flag = 0;
			client_tcp = tcp_accept(tcp);

			if (likely(client_tcp)) {
				// 创建一个子线程来处理发送接收数据的任务
				flag = pthread_create((pthread_t *)&threadinfo.threadid[threadinfo.counter], NULL, ReceiveData, (void *)client_tcp);
				RAISE_SYS_ERROR(flag);
				x_printf(D, "created thread successed, tid:%d", (int)threadinfo.threadid[threadinfo.counter]);
				threadinfo.counter += 1;
			} else {
				x_printf(D, "created thread failed");
				break;
			}
		}
	}
	CATCH
	{
		x_perror("accept error:%s", x_strerror(errno));
		tcp_destroy((struct tcp_socket *)client_tcp);
	}
	FINALLY
	{
		int i = 0;

		for (i = 0; i < threadinfo.counter; i++) {
			pthread_join((pthread_t)threadinfo.threadid[i], NULL);
			x_printf(D, "join thread id:%d", (int)threadinfo.threadid[i]);
		}
	}
	END;
}

void *ReceiveData(void *usr)
{
	struct tcp_socket volatile *client_tcp = (struct tcp_socket volatile *)usr;

	TRY
	{
		long threadid = GetThreadID();

		while (1) {
			int ret = -1;
			x_printf(D, "ReceiveData GetThreadID:%ld  fd:%d ", threadid, client_tcp->fd);
			// x_printf(D, "ReceiveData ThreadSelfID:%ld  fd:%d ", pthread_self(), client_tcp->fd);
			ret = tcp_read((struct tcp_socket *)client_tcp);

			if (likely(ret > 0)) {
				// 接收数据成功
				x_printf(D, "thread:%d received data:%.*s", (int)GetThreadID(), client_tcp->cache.end - client_tcp->cache.start, &client_tcp->cache.buff[client_tcp->cache.start]);

				if (unlikely(!strncmp("tcp->cache.buff[tcp->cache.start]", "EOF", strlen("EOF")))) {
					// 已经发送到文件的结尾 直接退出
					x_printf(D, LOG_D_COLOR "the end of file"LOG_COLOR_NULL);
					break;
				} else {
					// 回显数据，将客服发送过来的数据发送回给客户端
					x_printf(D, "start to send data");
					ret = tcp_write((struct tcp_socket *)client_tcp);

					if (likely(ret > 0)) {
						cache_cutmem((struct cache *)&client_tcp->cache);
						x_printf(D, "send data successed");
					} else {
						x_printf(D, "send data failed");
						break;
					}
				}
			} else if (likely(ret == 0)) {
				// 对端关闭
				x_printf(D, "thread: %ld client already closed", threadid);
				break;
			} else {
				// 发生错误
				break;
			}
		}
	}
	CATCH
	{
		x_perror("thread:%ld recevied data failed", GetThreadID());
	}
	FINALLY
	{
		tcp_destroy((struct tcp_socket *)client_tcp);
	}
	END;

	return NULL;
}

static void _deal_signal(int signo)
{
	// 主线程退出，子线程继续与客户端通信
	x_printf(D, "received sigal:%d", signo);
	// sleep(1);
}

