#include <stdio.h>
#include <stdlib.h>
#include "log.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "jtt_package.h"
#include "jtt_body.h"
#include "jtt_client.h"

#define GPS_PKG_LEN             (1024 * 5)	/*单个数据包的最大长度*/

#define ASYNC_CONNECT_TIME      10		// 异步connect时间
#define RECONNECT_SPACE         3		// 重连间隔时间，调整为3秒，是为了让客户处理故障后能及时获得数据
#define LOGON_RESP_SPACE        3		// 登录响应间隔时间

/*启动初始化*/
void async_user_init(BODY_USER *user)
{
	user->sockfd = -1;
	user->status.online = -1;
	user->status.conn_time = 0;
	user->status.logon_time = 0;
	user->status.logout_time = 0;
}

/*离线重置*/
void async_user_reset(BODY_USER *user)
{
	if (-1 != user->sockfd) {
		close(user->sockfd);
		user->sockfd = -1;
	}

	user->status.online = 0;
	user->status.conn_time = 0;
	user->status.logon_time = 0;
	time(&user->status.logout_time);
}

/*在线判断*/
int async_user_is_online(BODY_USER user)
{
	if (4 == user.status.online) {
		return 1;
	}

	return 0;
}

/*
 *   返回值:
 *   1：握手未完成
 *   0：握手完成
 *   -1：错误
 */
static int async_is_connect(int sockfd)
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
		return 1;
	}

	getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);

	if (0 == error) {
		return 0;
	}

	log_info(LOG_E, "connect握手错误\n");
	return -1;
}

/*
 *   返回值：
 *         1:登录未完成
 *         0:登录成功
 *        -1:错误
 */
int async_user_connect(PKG_USER *p_user, BODY_USER *b_user)
{
	int             i = 5;
	int             ret;
	time_t          tm;
	int             start = 0;
	unsigned char   pkg[GPS_PKG_LEN] = {};
	int             plen = 0;
	char            ip[16] = {};
	BODY_CONN_REQ   conn = {};
	BODY_CONN_RESP  conn_resp = {};
	PKG_BODY        pkg_body = {};
	struct timeval  tv;

	tv.tv_sec = 0;

	while (i--) {
		switch (b_user->status.online)
		{
			case -1:/*启动状态*/
			{
				start = 1;
				b_user->status.online = 0;
				break;
			}

			case  0:/*离线状态,去连接*/
			{
				time(&tm);

				/*重连判断*/
				if (!start && ((tm - b_user->status.logout_time) < RECONNECT_SPACE)) {
					log_info(LOG_D, "重连时间未到\n");
					return 1;
				}

				if (-1 == ip_get_from_domain(b_user->ip, ip, sizeof(ip))) {
					async_user_reset(b_user);
					log_info(LOG_E, "域名[%s]解析错\n", b_user->ip);
					return -1;
				}

				log_info(LOG_D, "域名[%s],ip[%s]\n", b_user->ip, ip);
				ret = jtt_connect(ip, b_user->port, &b_user->sockfd);

				if (-1 == ret) {
					async_user_reset(b_user);
					log_info(LOG_D, "链接失败online[%d]\n", b_user->status.online);
					return -1;
				}

				/*连接请求时间*/
				time(&b_user->status.conn_time);
				/*状态变化*/
				b_user->status.online = 1;
				break;
			}

			case  1:/*connect请求状态，握手判断*/
			{
				ret = async_is_connect(b_user->sockfd);

				if (ret == 0) {	/*完成握手*/
					b_user->status.online = 2;
					log_info(LOG_D, "connect 握手完成online[%d]\n", b_user->status.online);
					break;
				} else if (ret == 1) {	/*未完成握手*/
					time(&tm);

					if ((tm - b_user->status.conn_time) > ASYNC_CONNECT_TIME) {
						log_info(LOG_D, "握手超时online[%d]\n", b_user->status.online);
						async_user_reset(b_user);
						return -1;
					}

					log_info(LOG_D, "等待握手完成online[%d]\n", b_user->status.online);
					return 1;
				} else {/*握手错误*/
					async_user_reset(b_user);
					log_info(LOG_D, "握手错误online[%d]\n", b_user->status.online);
					return -1;
				}

				break;
			}

			case  2:/*connect完成握手状态，发送登录请求*/
			{
				tv.tv_usec = 500;
				body_conn_set(*b_user, &conn);
				pkg_body.body = &conn;
				pkg_body.msg_id = UP_CONNECT_REQ;
				pkg_body.blen = sizeof(conn);

				plen = sizeof(pkg);

				if (-1 == pkg_encode(p_user, pkg, &plen, pkg_body)) {
					async_user_reset(b_user);
					return -1;
				}

				log_info(LOG_I, "sockfd[%d]\n", b_user->sockfd);

				/*不是成功都表示失败*/
				if (0 != jtt_send(b_user->sockfd, pkg, plen, tv)) {
					async_user_reset(b_user);
					log_info(LOG_E, "jtt_send error\n");
					return -1;
				}

				b_user->status.online = 3;
				time(&b_user->status.logon_time);
				log_info(LOG_D, "登录请求发送成功\n");
				continue;
				break;
			}

			case  3:/*登录请求发送完成状态，接收登录响应*/
			{
				tv.tv_usec = 5000;
				plen = sizeof(pkg);
				memset(pkg, 0, plen);
				ret = jtt_recv(b_user->sockfd, pkg, &plen, tv);

				if (0 != ret) {
					log_info(LOG_D, "接受登录数据不成功online[%d]\n", b_user->status.online);
					time(&tm);

					if ((tm - b_user->status.logon_time) > LOGON_RESP_SPACE) {
						log_info(LOG_D, "接受登录数据超时\n");
						async_user_reset(b_user);
						return -1;
					} else {
						log_info(LOG_D, "等下次接受登录数据\n");
						return 1;
					}
				}

				if (-1 == pkg_decode(*p_user, pkg, &plen)) {
					async_user_reset(b_user);
					return -1;
				}

				memcpy(&conn_resp, pkg + sizeof(JTT_HEAD), sizeof(BODY_CONN_RESP));

				log_info(LOG_I, "登录响应码[%d]\n", conn_resp.result);

				if (0 != conn_resp.result) {
					async_user_reset(b_user);
					return -1;
				}

				b_user->status.online = 4;
				log_info(LOG_D, "登录成功\n");
				return 0;

				break;
			}

			case  4:/*登录成功状态*/
			{
				return 0;
			}

			default:/*不存在的状态标识*/
				log_info(LOG_E, "不存在的用户状态[%d]\n", b_user->status.online);
		}
	}

	return 0;
}

