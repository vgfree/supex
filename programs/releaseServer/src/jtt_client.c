#include "jtt_client.h"
#include <string.h>
#include "log.h"

#define OUT_WR_TRY_TIMES 3		// 发送数据时，写数据尝试次数

/*
 *   返回值:
 *         0：握手未完成
 *         1：握手完成
 *        -1：错误
 */
int is_ok_connect(int sockfd)
{
	int             error;
	int             len = sizeof(error);
	int             ret;
	fd_set          set;
	struct timeval  tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&set);
	FD_SET(sockfd, &set);

	/*仅判断状态FIXME*/
	ret = select(sockfd + 1, NULL, &set, NULL, &tv);

	if (0 == ret) {
		return 0;
	}

	getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);

	if (0 == error) {
		return 1;
	}

	log_info(LOG_E, "connect握手错误\n");
	return -1;
}

/*
 *   异步连接
 *   返回：
 *         0:
 *        -1:
 */
int jtt_connect(const char *ip, short port, int *sockfd)
{
	int                     ret;
	int                     so_reuseaddr = 1;
	struct sockaddr_in      addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	*sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (-1 == *sockfd) {
		log_info(LOG_E, "socket,strerror:%s\n", strerror(errno));
		return -1;
	}

	fcntl(*sockfd, F_SETFL, fcntl(*sockfd, F_GETFL) | O_NONBLOCK);
	setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));

	ret = connect(*sockfd, (struct sockaddr *)&addr, sizeof(addr));

	if ((-1 == ret) && (errno != EINPROGRESS)) {
		return -1;
	}

	return 0;
}

int jtt_disconnect(int sockfd)
{
	close(sockfd);

	return 0;
}

/*
 *   返回值：
 *         0: 成功
 *        -1: 错误
 *        -2: 超时
 */
static int sendn(int sockfd, unsigned char *buf, int len, struct timeval tv)
{
	int     ret;
	int     left;
	int     tms;
	fd_set  wfds;

	tms = OUT_WR_TRY_TIMES;		/*重复发送的次数限制*/
	left = len;

	while (left > 0 && tms--) {
		FD_ZERO(&wfds);
		FD_SET(sockfd, &wfds);
		ret = select(sockfd + 1, NULL, &wfds, NULL, &tv);

		if (-1 == ret) {
			log_info(LOG_E, "发送数据时,select 错,strerror:%s\n", strerror(errno));
			return -1;
		} else if (0 == ret) {
			log_info(LOG_W, "单次发送数据时,等待超时,strerror:%s\n", strerror(errno));
			continue;
		}

		ret = send(sockfd, buf + len - left, left, 0);

		if (-1 == ret) {
			if ((EAGAIN == errno) || (EINTR == errno)) {
				log_info(LOG_I, "EAGAIN or EINTR \n");
				continue;
			}

			log_info(LOG_E, "发送数据时,send错,strerror:%s\n", strerror(errno));
			return -1;
		}

		left -= ret;
	}

	if (-1 == tms) {
		log_info(LOG_W, "发送数据时,重复发送的次数超过限制\n");
		return -2;
	}

	return 0;
}

/*有多少发送多少*/
int jtt_send(int sockfd, unsigned char *buf, int len, struct timeval tv)
{
	int ret = 0;

	ret = sendn(sockfd, buf, len, tv);

	if (-1 == ret) {
		log_info(LOG_E, "send,strerror:%s\n", strerror(errno));
		return -1;
	} else if (-2 == ret) {
		log_info(LOG_W, "send,超时\n");
		return -2;
	}

	return 0;
}

static int recvn(int sockfd, unsigned char *buf, int *len, struct timeval tv)
{
	int     ret;
	fd_set  rfds;

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);

	ret = select(sockfd + 1, &rfds, NULL, NULL, &tv);

	if (-1 == ret) {
		log_info(LOG_E, "接收时,select 错,strerror:%s\n", strerror(errno));
		return -1;
	} else if (0 == ret) {
		log_info(LOG_W, "接收时,等待超时或状态无变化,strerror:%s\n", strerror(errno));
		return -2;
	}

	ret = recv(sockfd, buf, *len, 0);

	if (-1 == ret) {
		if ((EAGAIN == errno) || (EINTR == errno)) {
			log_info(LOG_D, "EAGAIN or EINTR \n");
		}

		log_info(LOG_E, "接收时,recv错,strerror:%s\n", strerror(errno));
		return -1;
	} else if (0 == ret) {
		log_info(LOG_E, "接收时,对方关闭链接,ret[%d],errno[%d],strerror:%s\n", ret, errno, strerror(errno));
		return -1;
	}

	*len = ret;
	return 0;
}

/*能接收多少是多少，不知接收数据的长度*/
int jtt_recv(int sockfd, unsigned char *buf, int *len, struct timeval tv)
{
	int ret = 0;

	ret = recvn(sockfd, buf, len, tv);

	if (-1 == ret) {
		log_info(LOG_F, "recv,strerror:%s\n", strerror(errno));
		return -1;
	} else if (-2 == ret) {
		log_info(LOG_W, "recv,超时\n");
		return -2;
	}

	return 0;
}

