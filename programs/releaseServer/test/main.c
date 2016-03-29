#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "jtt_package.h"
#include "jtt_body.h"
#include "jtt_comm.h"
#include "jtt_client.h"

int socket_init(int port)
{
	struct sockaddr_in      my_addr;
	int                     listenfd;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		exit(-1);
	}

	/* set nonblock */
	// fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL) | O_NONBLOCK);
	/* set reuseaddr */
	int so_reuseaddr = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = PF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;	// inet_addr("127.0.0.1");

	if (bind(listenfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		exit(-1);
	} else {}

	if (listen(listenfd, 10) == -1) {
		exit(-1);
	}

	return listenfd;
}

int main(void)
{
	int                     port = 6666;
	int                     listenfd;
	int                     fd;
	socklen_t               len;
	struct sockaddr_in      addr;

	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 1000;

	listenfd = socket_init(port);
	fd = accept(listenfd, (struct sockaddr *)&addr, &len);

	PKG_USER        user = {};
	PKG_BODY        body = {};
	unsigned char   pkg[2024];
	int             plen = sizeof(pkg);

	user.encrypt = 0;
	user.msg_enterid = 30;
	user.msg_sz = 0;
	user.head_flag = HEAD_FLAG;
	user.end_flag = END_FLAG;
	VERSION(user.version);

	// 接收登录
	unsigned char   buf[1024] = {};
	int             buflen;
	int             ret;
	BODY_CONN_REQ   conn;
	JTT_HEAD        head;

	// plen=sizeof(pkg);
	buflen = sizeof(buf);
	ret = recv(fd, buf, buflen, 0);

	if (-1 == ret) {
		printf("recv\n");
		return -1;
	}

	plen = ret;
	memcpy(pkg, buf, ret);

	printf("plen[%d],ret[%d]\n", plen, ret);
	pkg_decode(user, pkg, &plen);

	memcpy(&conn, pkg + sizeof(JTT_HEAD), sizeof(conn));
	printf("uid[%d],passwd[%s]\n", ntohl(conn.uid), conn.passwd);

	plen = sizeof(pkg);
	memset(pkg, 0, plen);

	BODY_CONN_RESP conn_resp;
	conn_resp.result = 0;
	conn_resp.verify_code = htonl(2);

	body.blen = sizeof(conn_resp);
	body.body = &conn_resp;
	body.msg_id = UP_CONNECT_RSP;
	pkg_encode(&user, pkg, &plen, body);
	ret = send(fd, pkg, plen, 0);
	printf("send-------------ret[%d],len[%d]\n", ret, plen);
	// 保持

	while (1) {
		printf("while\n");
		plen = sizeof(pkg);
		memset(pkg, 0, plen);

		ret = recv(fd, pkg, plen, 0);

		if (-1 == ret) {
			printf("recv ret[%d]strerror[%s]\n", ret, strerror(errno));
			return -1;
		} else if (0 == ret) {
			printf("close\n");
			return -1;
		}

		plen = ret;

		if (-1 == pkg_decode(user, pkg, &plen)) {
			printf("解码错\n");
			continue;
		}

		memcpy(&head, pkg, sizeof(head));

		if (ntohs(head.msg_id) == UP_LINKTEST_REQ) {
			printf("UP_LINKTEST_RSQ\n");

			int heart;
			body.body = &heart;
			body.blen = 0;
			body.msg_id = UP_LINKTEST_RSP;

			plen = sizeof(pkg);
			pkg_encode(&user, pkg, &plen, body);
			ret = send(fd, pkg, plen, 0);
			printf("收到 heart\n");
			continue;
		}

		// GPS
		BODY_GPS_REQ gps_req;
		memcpy(&gps_req, pkg + sizeof(head), sizeof(gps_req));
		printf("GPS:direction[%u]\n", ntohs(gps_req.body.direction));
		printf("imei[%s]\n", gps_req.head.vehicle_no);
		printf("lon[%u]\n", ntohl(gps_req.body.lon));
		printf("lat[%u]\n", ntohl(gps_req.body.lat));
	}

	/*
	 *        //补传
	 *        BODY_GPS_EXTRA_HEAD h_head;
	 *        plen = sizeof(pkg);
	 *        memset(pkg,0,plen);
	 *
	 *        ret = recv(fd,pkg,plen,0);
	 *   printf("recv len[%d]\n",ret);
	 *        if(-1 == ret){
	 *                printf("recv\n");
	 *                return -1;
	 *        }
	 *        plen = ret;
	 *
	 *        pkg_decode(user,pkg,&plen);
	 *        memcpy(&head,pkg,sizeof(head));
	 *        printf("msg_id[%d]\n",ntohs(head.msg_id));
	 *        memcpy(&h_head,pkg+sizeof(head),sizeof(h_head));
	 *        printf("imei[%s],count[%d],data_type[%x]\n",h_head.vehicle_no,h_head.count,ntohs(h_head.data_type));
	 *
	 *        int i=0;
	 *        BODY_GPS_BODY g_body;
	 *        //JTT_GPS_BODY g_body[2];
	 *   //	memcpy(&g_body[0],pkg+sizeof(head)+sizeof(h_head),sizeof(g_body));
	 *   //	memcpy(&g_body[1],pkg+sizeof(head)+sizeof(h_head)+sizeof(g_body),sizeof(g_body));
	 *   //	printf("gps[0]:direction[%d]\n",ntohs(g_body[0].direction));
	 *   //	printf("gps[1]:direction[%d]\n",ntohs(g_body[1].direction));
	 *
	 *        for(i=0;i<h_head.count;i++){
	 *                memcpy(&g_body,pkg+sizeof(head)+sizeof(h_head)+i*sizeof(g_body),sizeof(g_body));
	 *                printf("gps[%d]:direction[%d]\n",i,ntohs(g_body.direction));
	 *        }
	 *
	 */

	// 注销
	plen = sizeof(pkg);
	memset(pkg, 0, plen);

	ret = recv(fd, pkg, plen, 0);

	if (-1 == ret) {
		printf("recv\n");
		return -1;
	}

	plen = ret;

	pkg_decode(user, pkg, &plen);
	memcpy(&head, pkg, sizeof(head));

	if (UP_DISCONNECT_REQ != ntohs(head.msg_id)) {
		printf("error:not UP_DISCONNECT_REQ\n");
		return -1;
	}

	printf("UP_DISCONNECT_REQ\n");
	plen = sizeof(pkg);
	memset(pkg, 0, plen);
	int disconn;
	body.body = &disconn;
	body.blen = 0;
	body.msg_id = UP_DISCONNECT_RSP;

	pkg_encode(&user, pkg, &plen, body);
	send(fd, pkg, plen, 0);
	close(fd);

	close(listenfd);
	return 0;
}

