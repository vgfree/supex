/*
 *   ===============================================================
 *   模块说明：
 *        809协议通信模块
 *
 *
 *   ===============================================================
 */
#include <stdio.h>
#include "jtt_comm.h"
#include "log.h"
#include <assert.h>

#define HEART_COUNT             3	/*心跳探测次数*/
#define OUT_GPS_WAIT_TIME       5000	/*发送GPS数据的超时时间,单位us*/
#define OUT_HEART_WAIT_TIME     500	/*发送心跳包的超时时间，单位us*/

int jtt_real_send(PKG_USER *pkg_user, BODY_USER body_user, BODY_GPS *gps, int n)
{
	int             i;
	BODY_GPS_REQ    gps_req = {};
	PKG_BODY        pkg_body;
	unsigned char   pkg[GPS_PKG_LEN];
	int             plen;
	struct timeval  tv;

	tv.tv_sec = 0;
	tv.tv_usec = OUT_GPS_WAIT_TIME;

	assert(gps);

	for (i = 0; i < n; i++) {
		/*组包*/
		if (-1 == body_gps_set(&gps_req, &gps[i])) {
			return -1;
		}

		pkg_body.msg_id = UP_EXG_MSG;
		pkg_body.body = &gps_req;
		pkg_body.blen = sizeof(gps_req);

		plen = sizeof(pkg);
		memset(&pkg, 0, plen);

		if (-1 == pkg_encode(pkg_user, pkg, &plen, pkg_body)) {
			return -1;
		}

		/*发送,不检查超时*/
		if (-1 == jtt_send(body_user.sockfd, pkg, plen, tv)) {
			return -1;
		}
	}

	return 0;
}

int jtt_extra_send(PKG_USER *pkg_user, BODY_USER body_user, BODY_GPS *extra, int n)
{
	int             i;
	PKG_BODY        pkg_body = {};
	unsigned char   pkg[GPS_PKG_LEN] = {};
	int             plen;
	unsigned char   data[GPS_PKG_LEN] = {};
	int             dlen = sizeof(data);
	int             left;
	struct timeval  tv;

	tv.tv_sec = 0;
	tv.tv_usec = OUT_GPS_WAIT_TIME;

	assert(extra);

	left = n;
	log_info(LOG_D, "补传数据点数[%d]\n", n);

	while (left > 0) {
		/*协议规定，一个补传包最多5个点*/
		if (left >= 5) {
			i = 5;
		} else {
			i = left;
		}

		if (-1 == body_extra_gps_set(data, &dlen, extra + n - left, i)) {
			return -1;
		}

		pkg_body.msg_id = UP_EXG_MSG;
		pkg_body.body = &data;
		pkg_body.blen = dlen;

		plen = sizeof(pkg);
		memset(&pkg, 0, plen);

		if (-1 == pkg_encode(pkg_user, pkg, &plen, pkg_body)) {
			return -1;
		}

		/*发送,不检查超时*/
		if (-1 == jtt_send(body_user.sockfd, pkg, plen, tv)) {
			log_info(LOG_E, "发送补传数据失败\n");
			return -1;
		}

		left = left - i;
	}

	return 0;
}

/*
 *   说明：心跳失败不允许在内部close(sockfd)
 */
int jtt_heart(PKG_USER *pkg_user, BODY_USER *body_user)
{
	unsigned char   pkg[GPS_PKG_LEN] = {};
	int             plen = 0;
	PKG_BODY        pkg_body = {};
	int             heart;
	//	JTT_HEAD        head;
	int             ret;
	struct timeval  tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	assert(body_user);

	/*
	 *        如果不是第一个心跳包，则接收心跳响应(每次生命的第一个心跳包)
	 *        body_user->heart_cnt:表示没有收到的心跳响应包数,刚连接的初始值为0
	 */
	if (0 != body_user->heart_cnt) {
		plen = sizeof(pkg);
		memset(pkg, 0, plen);
		ret = jtt_recv(body_user->sockfd, pkg, &plen, tv);

		if ((-1 == ret) || (-2 == ret)) {
			log_info(LOG_I, "接收用户[%s]心跳响应失败\n", body_user->srv_id);

			if (HEART_COUNT == body_user->heart_cnt) {
				log_info(LOG_E, "3次接收用户[%s]心跳响应失败\n", body_user->srv_id);
				body_user->heart_cnt = 0;
				return -1;
			}

			body_user->heart_cnt++;
		} else {/*收到信息即为心跳，无需解析。1、对客户进行容错处理，2、合理减少处理逻辑*/
#if 0
			if (-1 == pkg_decode(*pkg_user, pkg, &plen)) {
				log_info(LOG_E, "心跳包解码错\n");
				body_user->heart_cnt = 0;
				return -1;
			}

			memcpy(&head, pkg, sizeof(head));

			if (ntohs(head.msg_id) != UP_LINKTEST_RSP) {
				log_info(LOG_E, "[%x] not UP_LINKTEST_RSP\n", ntohs(head.msg_id));
				body_user->heart_cnt = 0;
				return -1;
			}
#endif
			body_user->heart_cnt = 1;
		}
	} else {
		body_user->heart_cnt = 1;
	}

	pkg_body.body = &heart;
	pkg_body.blen = 0;
	pkg_body.msg_id = UP_LINKTEST_REQ;

	tv.tv_usec = OUT_HEART_WAIT_TIME;

	plen = sizeof(pkg);
	memset(pkg, 0, plen);

	if (-1 == pkg_encode(pkg_user, pkg, &plen, pkg_body)) {
		body_user->heart_cnt = 0;
		log_info(LOG_E, "加码心跳包失败\n");
		return -1;
	}

	/*超时不算失败*/
	if (-1 == jtt_send(body_user->sockfd, pkg, plen, tv)) {
		body_user->heart_cnt = 0;
		log_info(LOG_E, "发送心跳数据失败\n");
		return -1;
	}

	return 0;
}

