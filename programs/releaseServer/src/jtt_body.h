#ifndef __JTT_BODY_H__
#define __JTT_BODY_H__

#include "pub_incl.h"

#define UP_CONNECT_REQ                  0x1001
#define UP_CONNECT_RSP                  0x1002
#define UP_DISCONNECT_REQ               0x1003
#define UP_DISCONNECT_RSP               0x1004
#define UP_LINKTEST_REQ                 0x1005
#define UP_LINKTEST_RSP                 0x1006
#define UP_EXG_MSG                      0x1200
#define UP_EXG_MSG_REAL_LOCATION        0x1202
#define UP_EXG_MSG_HISTORY_LOCATION     0x1203

typedef struct _STATUS
{
	int     online;		// 在线状态，-1：启动，0：不在线，1：connect,2：connect完成,3：登录请求完成，4：登录成功，在线状态
	time_t  conn_time;	// online==2时的时间，链接时间
	time_t  logon_time;	// online==3时的时间，登录时间
	time_t  logout_time;	// online==0时的时间，离线时间
} STATUS;

typedef struct _BODY_USER
{
	STATUS          status;		// 用户状态信息
	struct timeval  sd_space;	// 向该用户发送数据的时间间隔，频率控制用
	struct timeval  last_space;	//上次发送时间
	char            srv_id[16 + 1];	// 本系统对对端系统的标志，用于识别
	//	time_t		last_heart;	//上次心跳时间
	int             heart_cnt;	// 没有收到的心跳响应包的数量

	char            ip[256];	// 对端服务器ip
	short           port;		// 对端服务器port
	int             sockfd;
	//	time_t		conn_time;	//connect握手成功后为0,connect但握手未完成时为connect的时间

	unsigned        uid;		// 登录对端使用的账号
	char            passwd[8];	// 登录对端使用的密码
} BODY_USER;

typedef struct _BODY_GPS
{
	char            imei[16];	// 终端编号,用作车牌
	char            date[4];	// 日期：dmyy
	char            time[3];	// 时间：hms
	unsigned        lon;		// 经度
	unsigned        lat;		// 纬度
	unsigned short  vec1;		// 卫星定位终端上传的速度，单位km/h
	short           direction;	// 方向，0-359,单位度，正北为0，顺时针
	unsigned short  altitude;	// 海拔高度，单位m
	unsigned        state;
} BODY_GPS;

/*协议数据体结构*/
#pragma pack(push,1)

typedef struct _BODY_CONN_REQ
{
	unsigned int    uid;		// userid,用户名
	char            passwd[8];	// 密码
	char            ip[32];		// 从链路IP
	unsigned short  port;		// 从链路port
} BODY_CONN_REQ;

typedef struct _BODY_CONN_RESP
{
	char            result;		// 结果：
	unsigned int    verify_code;	// 校验码
} BODY_CONN_RESP;

typedef struct _BODY_DISCONN_REQ
{
	unsigned int    uid;
	char            passwd[8];
} BODY_DISCONN_REQ;

typedef struct _BODY_GPS_HEAD
{
	char            vehicle_no[21];	// 车牌号
	char            vehicle_color;	// 车牌颜色
	unsigned short  data_type;	// 子业务类型标志
	unsigned        data_len;	// 后续数据长度
} BODY_GPS_HEAD;

typedef struct _BODY_GPS_BODY
{
	char            encrypt;	// 加密标志
	char            date[4];	// 日期：dmyy
	char            time[3];	// 时间：hms
	unsigned        lon;		// 经度
	unsigned        lat;		// 纬度
	unsigned short  vec1;		// 卫星定位终端上传的速度，单位km/h
	unsigned short  vec2;		// 车辆行驶记录设备上传的速度，单位km/h
	unsigned        vec3;		// 车辆当前总里程数，单位km
	unsigned short  direction;	// 方向，0-359,单位度，正北为0，顺时针
	unsigned short  altitude;	// 海拔高度，单位m
	unsigned        state;		// 车辆状态
	unsigned        alarm;		// 报警状态
} BODY_GPS_BODY;

typedef struct _BODY_GPS_REQ
{
	BODY_GPS_HEAD   head;
	BODY_GPS_BODY   body;
} BODY_GPS_REQ;

typedef struct _BODY_GPS_EXTRA_HEAD
{
	char            vehicle_no[21];	// 车牌号
	char            vehicle_color;	// 车牌颜色
	unsigned short  data_type;	// 子业务类型标志
	unsigned        data_len;	// 后续数据长度
	char            count;		// 1<=count<=5;后续BODY_GPS_BODY的个数
} BODY_GPS_EXTRA_HEAD;

#pragma pack(pop)

void body_conn_set(BODY_USER user, BODY_CONN_REQ *conn_req);

void body_disconn_set(BODY_USER user, BODY_DISCONN_REQ *disconn_req);

int body_gps_set(BODY_GPS_REQ *gps_req, BODY_GPS *gps);

int body_extra_gps_set(void *hgps_req, int *hlen, BODY_GPS gps[], int n);

/*根据域名获得ip*/
int ip_get_from_domain(char *domain, char *ip, int iplen);
#endif	/* ifndef __JTT_BODY_H__ */

