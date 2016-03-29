#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "hiredis.h"

#define MAX_TOKEN_CNT (16)

typedef struct tagGpsInfo
{
	char    tokenCode[11];
	char    *value;
	size_t  vlen;
} gpsInfo_t;

gpsInfo_t g_gpsinfo[MAX_TOKEN_CNT];

static struct option longopts[] = {
	{ "help",  no_argument,       NULL, '?' },
	{ "from",  required_argument, NULL, 'f' },
	{ "gps",   required_argument, NULL, 'g' },
	{ "url",   required_argument, NULL, 'u' },
	{ "stime", required_argument, NULL, 's' },
	{ "etime", required_argument, NULL, 'e' },
	{ NULL,    0,                 NULL, 0   }
};

static void usage(const char *execfile)
{
	printf("Usage: %s [OPTION...]\n\n", execfile);
	printf("  --from=127.0.0.1:7501      Set src tsdb host\n");
	printf("  --gps=127.0.0.1:7502       Set dst gps tsdb host\n");
	printf("  --url=127.0.0.1:7503       Set dst url tsdb host\n");
	printf("  --stime=1438419992         Set start time\n");
	printf("  --etime=1438419992         Set end time\n");
	printf("  --help                     Show this message\n\n");
}

int parse_host(const char *host, char *ip, unsigned short *port)
{
	char *p = NULL;

	if ((NULL == host) || (NULL == ip) || (NULL == port)) {
		return -1;
	}

	if (NULL == (p = strchr(host, ':'))) {
		return -1;
	}

	if (p == host) {
		return -1;
	}

	strncpy(ip, host, (size_t)(p - host));

	*port = (unsigned short)atoi(p + 1);

	if (0 == *port) {
		return -1;
	}

	return 0;
}

#define MAX_VALUE_LEN (5 * 1024 * 1024)
char g_value[MAX_VALUE_LEN];

int process_gps(redisReply *reply, gpsInfo_t *gps_info)
{
	int     i = 0;
	char    buf[32] = { 0 };
	char    tokenCode[11] = { 0 };
	size_t  cols_cnt = 0;

	if ((NULL == reply) || (reply->len <= 0) || (NULL == gps_info)) {
		return -1;
	}

	if (reply->len > 4194304) {
		return -1;
	}

	char *p = strchr(reply->str, '@');

	if (NULL == p) {
		return -1;
	}

	p++;

	memset(gps_info, 0, MAX_TOKEN_CNT * sizeof(gpsInfo_t));

	char    *value = g_value;
	size_t  vlen = 16;
	char    *q = reply->str + reply->len;
	int     is_first = 1;
	char    *row_start = &value[vlen];
	size_t  row_len = 0;
	size_t  row_cnt = 0;

	for (cols_cnt = 0; p < q; p++) {
		if (*p == '|') {
			cols_cnt++;

			if (cols_cnt == 11) {
				if (*(p + 1) == '|') {
					return -1;
				} else {
					if (is_first) {
						memcpy(gps_info[i].tokenCode, p + 1, 10);
						is_first = 0;
					} else {
						if (0 != memcmp(gps_info[i].tokenCode, p + 1, 10)) {
							int len = sprintf(buf, "%d*7@", (int)row_cnt);
							memcpy(value + 16 - len, buf, len);
							gps_info[i].value = value + 16 - len;
							gps_info[i].vlen = vlen - 16 + len - row_len;
							memmove(row_start + 16, row_start, row_len);
							row_cnt = 0;
							i++;
							value = row_start;
							vlen = 16 + row_len;
							row_start = &value[16];
							memcpy(gps_info[i].tokenCode, p + 1, 10);
						}
					}
				}
			} else if (cols_cnt == 12) {
				cols_cnt = 0;
				value[vlen++] = *p;
				row_cnt++;
				row_start = &value[vlen];
				row_len = 0;
				continue;
			}
		}

		if (((cols_cnt == 0) || (cols_cnt == 1)) || ((cols_cnt >= 6) && (cols_cnt <= 10))) {
			value[vlen++] = *p;
			row_len++;
		}
	}

	int len = sprintf(buf, "%d*7@", (int)row_cnt);
	memcpy(value + 16 - len, buf, len);
	gps_info[i].value = value + 16 - len;
	gps_info[i].vlen = vlen - 16 + len;
	i++;
	return i;
}

static int process_internal(const char *activeuser_prefix, const char *gps_prefix, redisContext *src_conn, redisContext *gps_conn, redisContext *url_conn, const char *tt)
{
	redisReply      *p = NULL;
	redisReply      *q = NULL;
	redisReply      *r = NULL;
	char            *imei = NULL;
	int             i = 0;

	size_t gps_size;

	p = redisCommand(src_conn, "GET %s:%s", activeuser_prefix, tt);

	if ((NULL == p) || (REDIS_REPLY_STRING != p->type) || (0 == p->len)) {
		if (NULL != p) {
			freeReplyObject(p);
		}

		printf("[WARNNING] GET %s:%s failed\n", activeuser_prefix, tt);
		return 0;
	}

	imei = strchr(p->str, '@');

	if (NULL == imei) {
		freeReplyObject(p);
		return -1;
	}

	for (imei += 1; imei < p->str + p->len; imei += 16) {
		do {
			if (NULL == gps_conn) {
				break;
			}

			q = redisCommand(src_conn, "GET %s:%b:%s", gps_prefix, imei, (size_t)15, tt);

			if ((NULL == q) || (REDIS_REPLY_STRING != q->type) || (0 == q->len)) {
				if (NULL != q) {
					freeReplyObject(q);
				}

				break;
			}

			int gps_size = 0;

			if ((gps_size = process_gps(q, g_gpsinfo)) < 0) {
				freeReplyObject(q);
				char imei_tmp[16];
				memcpy(imei_tmp, imei, 15);
				imei_tmp[15] = '\0';
				printf("[ERROR] process_gps(%s:%s:%s) failed!\n", gps_prefix, imei_tmp, tt);
				return -1;
			}

			freeReplyObject(q);

			for (i = 0; i < gps_size; ++i) {
				r = redisCommand(gps_conn, "SET %s:%b:%s:%s %b", gps_prefix, imei, (size_t)15, tt, g_gpsinfo[i].tokenCode, g_gpsinfo[i].value, g_gpsinfo[i].vlen);

				if ((NULL == r) || (2 != r->len) || (*((unsigned short *)"OK") != *((unsigned short *)r->str))) {
					char imei_tmp[16];
					memcpy(imei_tmp, imei, 15);
					imei_tmp[15] = '\0';
					freeReplyObject(r);
					printf("[ERROR] SET %s:%s:%s:%s failed!\n", gps_prefix, imei_tmp, tt, g_gpsinfo[i].tokenCode);
					return -1;
				}

				freeReplyObject(r);
			}
		} while (0);

		do {
			if (NULL == url_conn) {
				break;
			}

			q = redisCommand(src_conn, "GET URL:%b:%s", imei, (size_t)15, tt);

			if ((NULL == q) || (REDIS_REPLY_STRING != q->type) || (0 == q->len)) {
				if (NULL != q) {
					freeReplyObject(q);
				}

				break;
			}

			r = redisCommand(url_conn, "SET URL:%b:%s %b", imei, (size_t)15, tt, q->str, (size_t)q->len);

			if ((NULL == r) || (2 != r->len) || (*((unsigned short *)"OK") != *((unsigned short *)r->str))) {
				char imei_tmp[16];
				memcpy(imei_tmp, imei, 15);
				imei_tmp[15] = '\0';
				printf("[ERROR] SET URL:%s:%s failed!\n", imei_tmp, tt);
			}

			freeReplyObject(r);

			freeReplyObject(q);
		} while (0);
	}

	if (NULL != gps_conn) {
		q = redisCommand(gps_conn, "SET %s:%s %b", activeuser_prefix, tt, p->str, p->len);

		if ((NULL == q) || (2 != q->len) || (*((unsigned short *)"OK") != *((unsigned short *)q->str))) {
			printf("[ERROR] SET %s:%s failed!\n", activeuser_prefix, tt);
		}

		freeReplyObject(q);
	}

	if (NULL != url_conn) {
		q = redisCommand(url_conn, "SET %s:%s", activeuser_prefix, tt, p->str, p->len);

		if ((NULL == q) || (2 != q->len) || (*((unsigned short *)"OK") != *((unsigned short *)q->str))) {
			printf("[ERROR] SET %s:%s failed!\n", activeuser_prefix, tt);
		}

		freeReplyObject(q);
	}

	freeReplyObject(p);

	return 0;
}

#define process_activeuser(src_conn, gps_conn, url_conn, tt)    process_internal("ACTIVEUSER", "GPS", src_conn, gps_conn, url_conn, tt)
#define process_ractiveuser(src_conn, gps_conn, tt)             process_internal("RACTIVEUSER", "RGPS", src_conn, gps_conn, NULL, tt)

int main(int argc, char *argv[])
{
	char            src_ip[16] = { 0 };
	unsigned short  src_port = 0;
	redisContext    *src_conn = NULL;
	char            gps_ip[16] = { 0 };
	unsigned short  gps_port = 0;
	redisContext    *gps_conn = NULL;
	char            url_ip[16] = { 0 };
	unsigned short  url_port = 0;
	redisContext    *url_conn = NULL;
	int             stime = 0;
	int             etime = 0;
	int             tm_reverse = 0;
	int             ret = 0;
	int             c = 0;

	/* parse args */
	while ((c = getopt_long(argc, argv, "f:g:u:s:e:", longopts, NULL)) != EOF) {
		switch (c)
		{
			case 'f':
				ret = parse_host(optarg, src_ip, &src_port);

				if (ret < 0) {
					printf("[ERROR] arg --from=%s err!\n\n", optarg);
					usage(argv[0]);
					return -1;
				}

				break;

			case 'g':
				ret = parse_host(optarg, gps_ip, &gps_port);

				if (ret < 0) {
					printf("[ERROR] arg --gps=%s err!\n\n", optarg);
					usage(argv[0]);
					return -1;
				}

				break;

			case 'u':
				ret = parse_host(optarg, url_ip, &url_port);

				if (ret < 0) {
					printf("[ERROR] arg --url=%s err!\n\n", optarg);
					usage(argv[0]);
					return -1;
				}

				break;

			case 's':
				stime = atoi(optarg);

				if (stime == 0) {
					printf("[ERROR] arg --stime=%s err!\n\n", optarg);
					usage(argv[0]);
					return -1;
				}

				break;

			case 'e':
				etime = atoi(optarg);

				if (etime == 0) {
					printf("[ERROR] arg --etime=%s err!\n\n", optarg);
					usage(argv[0]);
					return -1;
				}

				break;

			default:
				usage(argv[0]);
				return -1;
		}
	}

	/* check args */
	if (argc < 2) {
		usage(argv[0]);
		return -1;
	}

	if (src_port == 0) {
		usage(argv[0]);
		return -1;
	}

	if ((gps_port == 0) && (url_port == 0)) {
		printf("[ERROR] You have to choose one from gps and url at least for migration.\n");
		usage(argv[0]);
		return -1;
	}

	if ((stime == 0) || (etime == 0)) {
		printf("[ERROR] stime or etime err!\n");
		usage(argv[0]);
		return -1;
	}

	src_conn = redisConnect(src_ip, (int)src_port);

	if ((NULL == src_conn) || src_conn->err) {
		printf("[ERROR] Connect to src tsdb err!\n");
		return -1;
	}

	if (gps_port != 0) {
		gps_conn = redisConnect(gps_ip, (int)gps_port);

		if ((NULL == gps_conn) || gps_conn->err) {
			printf("[ERROR] Connect to gps tsdb err!\n");
			return -1;
		}
	}

	if (url_port != 0) {
		url_conn = redisConnect(url_ip, (int)url_port);

		if ((NULL == url_conn) || url_conn->err) {
			printf("[ERROR] Connect to url tsdb err!\n");
			return -1;
		}
	}

	if (stime > etime) {
		tm_reverse = 1;
	}

	for (;; ) {
		if (tm_reverse) {
			if (stime < etime) {
				break;
			}
		} else {
			if (stime > etime) {
				break;
			}
		}

		/* generate time interval. */
		char    tt[32] = { 0 };
		time_t  tm = (time_t)stime;
		strftime(tt, sizeof(tt), "%Y%m%d%H%M", localtime(&tm));
		tt[strlen(tt) - 1] = 0;

		if (tm_reverse) {
			stime -= 600;
		} else {
			stime += 600;
		}

		if (process_activeuser(src_conn, gps_conn, url_conn, tt) < 0) {
			break;
		}

		if (process_ractiveuser(src_conn, gps_conn, tt) < 0) {
			break;
		}
	}

__ERR:

	if (NULL != src_conn) {
		redisFree(src_conn);
	}

	if (NULL != gps_conn) {
		redisFree(gps_conn);
	}

	if (NULL != url_conn) {
		redisFree(url_conn);
	}

	return 0;
}

