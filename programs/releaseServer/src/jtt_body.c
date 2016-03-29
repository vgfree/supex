#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "jtt_body.h"
#include "log.h"

/*根据域名获得ip*/
int ip_get_from_domain(char *domain, char *ip, int iplen)
{
	struct hostent          *hp;
	struct in_addr          in;
	struct sockaddr_in      addr;

	if (!(hp = gethostbyname(domain))) {
		printf("can't resolve host\n");
		return -1;
	}

	memcpy(&addr.sin_addr.s_addr, hp->h_addr, 4);
	in.s_addr = addr.sin_addr.s_addr;
	strncpy(ip, inet_ntoa(in), iplen);

	return 0;
}

/*封装登录数据体*/
void body_conn_set(BODY_USER user, BODY_CONN_REQ *conn_req)
{
	assert(conn_req);

	conn_req->uid = htonl(user.uid);
	memcpy(conn_req->passwd, user.passwd, 8);

	memset(conn_req->ip, 0, 32);
	conn_req->port = 0x00;
}

/*封装断链数据体*/
void body_disconn_set(BODY_USER user, BODY_DISCONN_REQ *disconn_req)
{
	assert(disconn_req);

	disconn_req->uid = htonl(user.uid);
	memcpy(disconn_req->passwd, user.passwd, 8);
}

/*封装gps数据*/
int body_gps_set(BODY_GPS_REQ *gps_req, BODY_GPS *gps)
{
	assert(gps_req && gps);

	BODY_GPS_HEAD   *gps_head = &gps_req->head;
	BODY_GPS_BODY   *gps_body = &gps_req->body;

	/*GPS头*/
	gps_head->vehicle_color = 0x00;
	gps_head->data_type = htons(UP_EXG_MSG_REAL_LOCATION);
	gps_head->data_len = htonl(sizeof(BODY_GPS_BODY));

	/*GPS 数据*/
	memset(gps_head->vehicle_no, 0, 21);
	strcpy(gps_head->vehicle_no, gps->imei);	// 将imei作为车牌
	memcpy(gps_body->date, gps->date, 4);		/*时间日期的字节序转换是在外部进行的*/
	memcpy(gps_body->time, gps->time, 3);
	gps_body->lon = htonl(gps->lon);
	gps_body->lat = htonl(gps->lat);
	gps_body->vec1 = htons(gps->vec1);
	gps_body->direction = htons(gps->direction);
	gps_body->altitude = htons(gps->altitude);
	gps_body->state = htonl(gps->state);

	/*数据包中没有以下字段，清零*/
	gps_body->encrypt = 0x00;
	gps_body->vec2 = 0x00;
	gps_body->vec3 = 0x00;
	gps_body->alarm = 0x00;

	return 0;
}

/*封装补传数据*/
int body_extra_gps_set(void *hgps_req, int *hlen, BODY_GPS gps[], int n)
{
	assert(hgps_req && hlen && gps);

	BODY_GPS_EXTRA_HEAD     head = {};
	BODY_GPS_BODY           gps_body = {};
	int                     body_len = sizeof(gps_body);
	int                     len = sizeof(BODY_GPS_EXTRA_HEAD);
	void                    *ptr = hgps_req;
	int                     i;
	//	int			direction;

	if ((n <= 0) || (n > 5) || (len > *hlen)) {
		log_info(LOG_E, "参数错n[%d],len[%d]>hlen[%d]\n", n, len, *hlen);
		return -1;
	}

	/*数据体的头*/
	strcpy(head.vehicle_no, gps[0].imei);
	head.vehicle_color = 0x00;
	head.data_type = htons(UP_EXG_MSG_HISTORY_LOCATION);
	head.data_len = htonl(1 + n * sizeof(BODY_GPS_BODY));
	head.count = n;
	memcpy(ptr, &head, len);

	/*体*/
	for (i = 0; i < n; i++) {
		if (len + body_len > *hlen) {
			log_info(LOG_E, "输入buf空间太小\n");
			return -1;
		}

		memset(&gps_body, 0, body_len);
		memcpy(gps_body.date, gps[i].date, 4);
		memcpy(gps_body.time, gps[i].time, 3);
		gps_body.lon = htonl(gps[i].lon);
		gps_body.lat = htonl(gps[i].lat);
		gps_body.vec1 = htons(gps[i].vec1);
		//		direction		= gps[i].direction;
		//		if(-1 == direction)
		//			direction = 0;
		gps_body.direction = htons(gps[i].direction);
		gps_body.altitude = htons(gps[i].altitude);
		gps_body.state = htonl(gps->state);

		/*数据包中没有以下字段，清零*/
		gps_body.encrypt = 0x00;
		gps_body.vec2 = 0x00;
		gps_body.vec3 = 0x00;
		gps_body.alarm = 0x00;

		memcpy(ptr + len, &gps_body, body_len);
		len += body_len;
	}

	*hlen = len;

	return 0;
}

