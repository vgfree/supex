/*
 * auth: coanor
 * date: Sat Aug  3 10:20:26 CST 2013
 */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>

#include "custom_hash.h"

#define g_pi 3.1415926

// utils
static char char_to_num(char ch)
{
	unsigned char c;

	if ((ch >= '0') && (ch <= '9')) {
		c = ch - '0';
	} else {
		c = toupper(ch) - 'A' + 10;
	}

	return c;
}

static char *url_encode(const char *src)
{
	char            *out = calloc(1, strlen(src) * 3 + 1);
	char            *p = out;
	const char      *q = src;
	char            buf[4] = { 0 };

	while (*q != '\0') {
		switch (*q)
		{
			case '@':
			case '#':
			case '$':
			case '%':
			case '^':
			case '&':
			case '+':
			case '|':
			case '}':
			case '{':
			case '"':
			case ':':
			case '?':
			case '>':
			case '<':
			case '[':
			case ']':
			case '\\':
			case '\'':
			case ';':
			case '/':
			case ',':
			case ' ':
			case '`':
			case '\r':
			case '\n':
				sprintf(buf, "%%%02X", *q);
				strcat(p, buf);
				p += 3;
				++q;
				break;

			default:

				if ((unsigned char)*q > 0x7f) {
					/* unicode */
					sprintf(buf, "%%%X", (unsigned char)*q);
					strcat(p, buf);
					p += 3;
					++q;
				} else {
					*p = *q;
					++p;
					++q;
				}

				break;
		}
	}

	return out;
}

static char *url_decode(const char *src)
{
	char            *out = calloc(1, strlen(src) + 1);
	const char      *p = src;
	char            *q = out;

	char byte1, byte2;

	while (*p != '\0') {
		switch (*p)
		{
			case '%':
				byte1 = char_to_num(p[1]);
				byte2 = char_to_num(p[2]);
				*q = ((byte1 << 4) | byte2);
				p += 3;
				++q;
				break;

			default:
				*q = *p;
				++q;
				++p;
		}
	}

	return out;
}

// api
static int lua_url_encode_C(lua_State *L)
{
	const char      *src = luaL_checkstring(L, 1);
	char            *out = url_encode(src);

	lua_pushstring(L, out);
	free(out);
	return 1;
}

// api
static int lua_url_decode_C(lua_State *L)
{
	const char      *src = luaL_checkstring(L, -1);
	char            *out = url_decode(src);

	lua_pushstring(L, out);
	free(out);
	return 1;
}

// api
static int lua_msleep_C(lua_State *L)
{
	long ms = luaL_checknumber(L, 1);

	usleep(1000 * ms);
	return 0;
}

/* 得到中央经线经度
 */
static double GetCentralMeridian(double dL)
{
	double  dBeltNumber = floor((dL + 1.5) / 3.0);
	double  dL0 = dBeltNumber * 3.0;

	return dL0;
}

/* 计算地球上两点之间的距离
 *   / <param name="dLongitude1">第一点的经度</param>
 *   / <param name="dX1">第一点的X值</param>
 *   / <param name="dY1">第一点的Y值</param>
 *   / <param name="dLongitude2">第二点的经度</param>
 *   / <param name="dX2">第二点的X值</param>
 *   / <param name="dY2">第二点的Y值</param>
 */
static double CalculateTwoPointsDistance(double dLongitude1, double dX1, double dY1,
	double dLongitude2, double dX2, double dY2)
{
	double  dL01 = GetCentralMeridian(dLongitude1);
	double  dL02 = GetCentralMeridian(dLongitude2);
	double  dCentralDelta = fabs(dL02 - dL01);
	double  dDeltaX = 0.0;

	if (dLongitude2 > dLongitude1) {
		dDeltaX = dX2 - dX1 + dCentralDelta * 100 * 954;
	} else {
		dDeltaX = dX1 - dX2 + dCentralDelta * 100 * 954;
	}

	double dDistance = sqrt(pow(dDeltaX, 2) + pow(dY2 - dY1, 2));
	return dDistance;
}

/* 根据经纬度转换为投影坐标
 */
static void BL2XY(double dB, double dL, double *dX, double *dY)
{
	double  dBeltNumber = floor((dL + 1.5) / 3.0);
	double  dL0 = dBeltNumber * 3.0;

	double  a = 6378137;
	double  b = 6356752.3142;
	double  k0 = 1;
	double  FE = 500000;

	double  e1 = sqrt(1 - pow(b / a, 2));
	double  e2 = sqrt(pow(a / b, 2) - 1);

	dB = (dB * g_pi) / 180.0;
	double T = pow(tan(dB), 2);

	double C = e2 * e2 * pow(cos(dB), 2);

	dL = dL * g_pi / 180.0;
	dL0 = dL0 * g_pi / 180.0;
	double A = (dL - dL0) * cos(dB);

	double M = (1 - pow(e1, 2) / 4.0 - 3.0 * pow(e1, 4) / 64.0 - 5.0 * pow(e1, 6) / 256.0) * dB;
	M = M - (3.0 * pow(e1, 2) / 8.0 + 3.0 * pow(e1, 4) / 32.0 + 45.0 * pow(e1, 6) / 1024.0) * sin(dB * 2);
	M = M + (15.0 * pow(e1, 4) / 256.0 + 45.0 * pow(e1, 6) / 1024.0) * sin(dB * 4);
	M = M - (35.0 * pow(e1, 6) / 3072.0) * sin(dB * 6);
	M = a * M;

	double N = a / pow(1.0 - pow(e1, 2) * pow(sin(dB), 2), 0.5);

	double  mgs1 = pow(A, 2) / 2.0;
	double  mgs2 = pow(A, 4) / 24.0 * (5.0 - T + 9.0 * C + 4.0 * pow(C, 2));
	double  mgs3 = pow(A, 6) / 720.0 * (61.0 - 58.0 * T + pow(T, 2) + 270 * C - 330.0 * T * C);
	*dY = M + N * tan(dB) * (mgs1 + mgs2) + mgs3;
	*dY = *dY * k0;

	mgs1 = A + (1.0 - T + C) * pow(A, 3) / 6.0;
	mgs2 = (5.0 - 18.0 * T + pow(T, 2) + 14.0 * C - 58.0 * T * C) * pow(A, 5) / 120.0;
	*dX = (mgs1 + mgs2) * N * k0 + FE;
}

// api
static int bl2xy(lua_State *ls)
{
	double  lon = luaL_checknumber(ls, 1);
	double  lat = luaL_checknumber(ls, 2);

	double x, y;

	BL2XY(lat, lon, &x, &y);

	lua_pushnumber(ls, x);
	lua_pushnumber(ls, y);

	return 2;
}

// api
// thre return value's unit is mile
static int dist_C(lua_State *ls)
{
	double  lon1 = luaL_checknumber(ls, 1);
	double  lat1 = luaL_checknumber(ls, 2);
	double  lon2 = luaL_checknumber(ls, 3);
	double  lat2 = luaL_checknumber(ls, 4);

	// 2014-06-09 更新球面2点距离算法
	double dist = 6378137 * acos(sin(lat1 / 57.2958) * sin(lat2 / 57.2958) + cos(lat1 / 57.2958) * cos(lat2 / 57.2958) * cos((lon1 - lon2) / 57.2958));

	/*
	 *   double dx1, dy1;
	 *   double dx2, dy2;
	 *
	 *   BL2XY(lat1, lon1, &dx1, &dy1);
	 *   BL2XY(lat2, lon2, &dx2, &dy2);
	 *   double dist = CalculateTwoPointsDistance(lon1, dx1, dy1, lon2, dx2, dy2);
	 */

	lua_pushnumber(ls, dist);
	return 1;
}

#define TCP_ERR_CONNECT         -1
#define TCP_ERR_SEND            -2
#define TCP_ERR_SOCKOPT         -3
#define TCP_ERR_MEMORY          -4
#define TCP_ERR_TIMEOUT         -5

#define DEFAULT_RECV_SIZE       (32 * 1024)

static inline int _connect_server(const char *host, int client_port)
{
	int             sockfd = 0;
	struct timeval  timeout;

	timeout.tv_sec = 0;		// 0秒
	timeout.tv_usec = 500000;	// 0.5秒
	struct linger quick_linger;
	quick_linger.l_onoff = 1;
	quick_linger.l_linger = 0;

	unsigned long inaddr = inet_addr(host);

	if (inaddr != INADDR_NONE) {
		/* normal IP */
		struct sockaddr_in ad;
		memset(&ad, 0, sizeof(ad));
		ad.sin_family = AF_INET;

		memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
		ad.sin_port = htons(client_port);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if (sockfd < 0) {
			return -1;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1) {
			perror("setsockopt()");
			close(sockfd);
			return -1;
		}

		if (connect(sockfd, (struct sockaddr *)&ad, sizeof(ad)) == 0) {
			return sockfd;
		}

		setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
		close(sockfd);
		return -1;
	} else {
		char            port[6];/* 65535 */
		struct addrinfo hints, *res, *ressave;

		snprintf(port, 6, "%d", client_port);
		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		/* not IP, host name */
		if (getaddrinfo(host, port, &hints, &res) != 0) {
			return -1;
		}

		ressave = res;	/*must not NULL*/

		do {
			sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

			if (sockfd < 0) {
				continue;	/* ignore this one */
			}

			if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1) {
				perror("setsockopt()");
			} else {
				if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
					freeaddrinfo(ressave);
					return sockfd;
				}
			}

			setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
			close(sockfd);
		} while ((res = res->ai_next) != NULL);

		freeaddrinfo(ressave);
		return -1;
	}
}

static inline int http_get_body_length(char *buf)
{
	char *p = strstr(buf, "Content-length:");

	if (!p) {
		p = strstr(buf, "content-Length:");
	}

	if (!p) {
		p = strstr(buf, "Content-Length:");
	}

	if (!p) {
		p = strstr(buf, "content-length:");
	}

	if (!p) {
		return 0;
	}

	char *rn = strstr(p, "\r\n");

	if (!rn) {
		return 0;
	}

	char *d = strstr(p, " ");

	if (!d) {
		d = strstr(p, ":");
	}

	if (!d) {
		return 0;
	}

	return atol(d);
}

static inline int http_get_head_length(char *buf)
{
	char *p = strstr(buf, "\r\n\r\n");

	if (!p) {
		return 0;
	}

	return p - buf + 4;
}

int sync_tcp_ask(const char *host, short port, const char *data, size_t size, char **back, size_t time, int rule, int max_content_length)
{
	struct linger quick_linger;

	quick_linger.l_onoff = 1;
	quick_linger.l_linger = 0;

	/*
	 * struct linger delay_linger;
	 * delay_linger.l_onoff = 1;
	 * delay_linger.l_linger = 1;
	 */

	int sock = _connect_server(host, port);

	if (sock == -1) {
		// printf("connect server(%s:%d) fail", host, port);
		return TCP_ERR_CONNECT;
	}

	size_t  io_size = 0;
	int     bytes = 0;

	// printf("out data:%s\n", data);
	while (size != io_size) {
		bytes = send(sock, data + io_size,
				size - io_size,
				0);

		// printf("-------> send = %d\n",  bytes);
		if (bytes < 0) {
			// printf("discard all\n");
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
			close(sock);
			return TCP_ERR_SEND;
		}

		io_size += bytes;
	}

	if (!back) {
		close(sock);
		return 0;
	}

	if (time > 0) {
		struct timeval timeout;
		timeout.tv_sec = time / 1000000;
		timeout.tv_usec = time % 1000000;

		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1) {
			close(sock);
			// printf("can't set recv timeout!\n");
			return TCP_ERR_SOCKOPT;
		}
	}

	int     head_size = 0;
	int     body_size = 0;

	char    temp[DEFAULT_RECV_SIZE] = { 0 };
	size_t  max_size = DEFAULT_RECV_SIZE;

	char    *p_now = temp;
	char    *p_new = NULL;
	bytes = 0;
	io_size = 0;

	while (1) {
		bytes = recv(sock, p_now + io_size, max_size - io_size, 0);

		if (bytes > 0) {
			io_size += bytes;
			// printf("in data:%d\n", bytes);
		} else if (bytes == 0) {/* socket has closed when read after */
			// printf("remote socket closed!socket fd: %d\n", sock);
			break;
		} else {
			if (errno == EINTR) {
				continue;
			}

			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
				close(sock);

				if (p_now != temp) {
					free(p_now);
				}

				// printf("Time out!\n");
				return TCP_ERR_TIMEOUT;
			} else {/* socket is going to close when reading */
				// printf("ret :%d ,close socket fd : %d\n", bytes, sock);
				break;
			}
		}

		if (rule) {
			if (head_size <= 0) {
				head_size = http_get_head_length(p_now);

				if (head_size) {
					body_size = http_get_body_length(p_now);

					if ((max_content_length > 0) && (body_size >= max_content_length)) {
						// limit content_lenth Modify by jiang z.s. 2015-01-22
						setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
						close(sock);

						if (p_now != temp) {
							free(p_now);
						}

						return TCP_ERR_MEMORY;
					}
				}
			}

			if (head_size && (io_size >= head_size + body_size)) {
				// printf(" recv finished, auto disconnect server \n" ) ;
				break;
			}
		}

		/* reach default receive buffer size */
		if (io_size + 32 >= max_size) {
			max_size *= 2;
			p_new = calloc(max_size, 1);

			if (p_new == NULL) {
				setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
				close(sock);

				if (p_now != temp) {
					free(p_now);
				}

				// printf("no more memory!\n");
				return TCP_ERR_MEMORY;
			}

			memcpy(p_new, p_now, io_size);
			p_now = p_new;
		}
	}

	if (p_now == temp) {
		p_now = calloc(io_size, 1);

		if (p_now == NULL) {
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
			close(sock);
			// printf("no more memory!\n");
			return TCP_ERR_MEMORY;
		}

		memcpy(p_now, temp, io_size);
	}

	setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
	close(sock);

	if (io_size > 0) {
		*back = p_now;
	}

	return io_size;
}

int http_request_C(lua_State *L)
{
	/* check these parameters */
	size_t          rule = 0;
	size_t          limit_content_length = 0;
	size_t          size = luaL_checkint(L, 4);
	const char      *data = luaL_checkstring(L, 3);
	short           port = (short)luaL_checkint(L, 2);
	const char      *host = luaL_checkstring(L, 1);

	if (lua_gettop(L) == 5) {
		rule = luaL_checkint(L, 5);
	}

	if (lua_gettop(L) == 6) {
		rule = luaL_checkint(L, 5);
		limit_content_length = luaL_checkint(L, 6);
	}

	char *back = NULL;
	// printf("L backuse is %p\n", L);
	int ok = sync_tcp_ask(host, port, data, size, &back, 2800000, rule, limit_content_length);	// 2.8秒

	if (ok > 0) {
		lua_pushboolean(L, 1);
		lua_pushlstring(L, back, ok);
		free(back);
	} else {
		lua_pushboolean(L, 0);
		switch (ok)
		{
			case TCP_ERR_CONNECT:
				lua_pushfstring(L, "connect %s:%d failed!", host, port);
				break;

			case TCP_ERR_SEND:
				lua_pushfstring(L, "send fail!");
				break;

			case TCP_ERR_SOCKOPT:
				lua_pushstring(L, "can't set recv timeout!");
				break;

			case TCP_ERR_MEMORY:
				lua_pushstring(L, "no more memory!");
				break;

			case TCP_ERR_TIMEOUT:
				lua_pushstring(L, "time out!");
				break;

			default:
				lua_pushstring(L, "unknow error!");
				break;
		}
	}

	return 2;
}

static int create_uuid_C(lua_State *L)
{
	char    str[37] = { 0 };
	uuid_t  uuid;

	uuid_generate_time(uuid);
	uuid_unparse(uuid, str);

	lua_pushstring(L, str);

	return 1;
}

static int cutils_custom_hash(lua_State *L)
{
	if (lua_gettop(L) != 3) {
		lua_pushnumber(L, -1);
		return 1;
	}

	const char      *accountid = luaL_checkstring(L, 1);
	const int       server_num = (int)luaL_checkint(L, 2);
	const int       suffix = (int)luaL_checkint(L, 3);

	int hash_val = custom_hash_dist((char *)accountid, (int)server_num, suffix);
	lua_pushnumber(L, hash_val);
	return 1;
}

static size_t lua_domain_to_ip_address(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		lua_pushnil(L);
		lua_pushstring(L, "args count is error");
		return 2;
	}

	const char *host = luaL_checkstring(L, 1);

	unsigned long inaddr = inet_addr(host);

	if (inaddr != INADDR_NONE) {
		lua_pushstring(L, host);
		lua_pushstring(L, "");
		return 2;
	} else {
		struct addrinfo hints, *res, *ressave;

		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		/* not IP, host name */
		if (getaddrinfo(host, NULL, &hints, &res) != 0) {
			lua_pushnil(L);
			lua_pushfstring(L, "getaddrinfo failed, errno: %d ", errno);
			return 2;
		}

		char ip_address[16] = { 0 };
		inet_ntop(AF_INET, &(((struct sockaddr_in *)(res->ai_addr))->sin_addr), ip_address, 16);

		lua_pushstring(L, ip_address);
		lua_pushstring(L, "");
		return 2;
	}
}

static const luaL_Reg lib[] = {
	{ "url_encode",       lua_url_encode_C         },
	{ "url_decode",       lua_url_decode_C         },
	{ "msleep",           lua_msleep_C             },
	{ "gps_distance",     dist_C                   },
	{ "bl2xy",            bl2xy                    },
	// {"http", http_request_C},
	{ "uuid",             create_uuid_C            },
	{ "custom_hash",      cutils_custom_hash       },
	{ "domain2ipaddress", lua_domain_to_ip_address },
	{ NULL,               NULL                     }
};

int luaopen_cutils(lua_State *L)
{
	luaL_register(L, "cutils", lib);

	return 0;
}

