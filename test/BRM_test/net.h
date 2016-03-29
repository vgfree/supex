#ifndef         __NET_H_
#define         __NET_H_

#define         unblock(fd)			   \
	({					   \
		bool _ret = false;		   \
		_ret = set_sockfd(fd, O_NONBLOCK); \
		_ret;				   \
	})

#define         reuseaddr(fd)					     \
	({							     \
		bool _ret = false;				     \
		_ret = set_sockopt(fd, SOL_SOCKET, SO_REUSEADDR, 1); \
		_ret;						     \
	})

#define         LISTENQ 1024

int tcp_listen(const char *host, const char *service);

bool set_sockopt(int sockfd, int level, int optname, int optval);

bool set_sockfd(int sockfd, int flag);
#endif	/* ifndef __NET_H_ */

