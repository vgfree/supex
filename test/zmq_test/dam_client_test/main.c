//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/9/17.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include <ev.h>
#include <zmq.h>
#include "libmini.h"

struct pkgdt
{
	long    sec;
	int     microsec;
	char    seq[20];
};

struct hostaddr
{
	uint16_t        port;
	char            host[IPADDR_MAXSIZE];
};

static void setzmqimmediate(void *skt)
{
	int     flag = 0;
	int     opt = 1;
	size_t  vsize = sizeof(opt);

	flag = zmq_setsockopt(skt, ZMQ_IMMEDIATE, &opt, vsize);
	RAISE_SYS_ERROR(flag);
}

static void rcvdata(void *skt);

static void sndrequest(void *skt, struct hostaddr *host);

int main(int argc, char **argv)
{
	void    *ctx = NULL;
	void    *reqskt = NULL;
	void    *rcvskt = NULL;

	if (unlikely(argc != 5)) {
		x_printf(E, "Usage %s <reqip> <#reqport> <rcvdataip> <#rcvdataport>", argv[0]);
		return EXIT_FAILURE;
	}

	TRY
	{
		int             flag = 0;
		char            ipaddr[32];
		struct hostaddr host = {};

		SLogOpen(argv[0], NULL);

		ctx = zmq_ctx_new();
		assert(ctx);

		/* ----------------------               */
		/* 绑定本地端口，用于接收数据*/

		x_printf(D, "initialize the socket of receiver");
		rcvskt = zmq_socket(ctx, ZMQ_PULL);
		AssertRaise(rcvskt, EXCEPT_SYS);

		snprintf(ipaddr, sizeof(ipaddr), "tcp://%s:%d", argv[3], atoi(argv[4]));
		flag = zmq_bind(rcvskt, ipaddr);
		RAISE_SYS_ERROR(flag);

		/* ----------------------               */
		/* 连接远端，发送本地绑定的ip地址（不能是通用地址）和端口，告知对端本端接收数据的工作已就绪。*/

		x_printf(D, "initialize the socket of requester");
		reqskt = zmq_socket(ctx, ZMQ_PUSH);
		AssertRaise(reqskt, EXCEPT_SYS);

		/*其他选项*/
		setzmqimmediate(reqskt);

		snprintf(ipaddr, sizeof(ipaddr), "tcp://%s:%d", argv[1], atoi(argv[2]));
		flag = zmq_connect(reqskt, ipaddr);
		RAISE_SYS_ERROR(flag);

		/* ----------------------               */

		host.port = htons(atoi(argv[4]));
		snprintf(host.host, sizeof(host.host), "%s", argv[3]);

		sndrequest(reqskt, &host);
		/* ----------------------               */

		rcvdata(rcvskt);
	}
	CATCH
	{}
	FINALLY
	{}
	END;

	return EXIT_SUCCESS;
}

static void rcvdata(void *skt)
{
	int             byte = 0;
	struct pkgdt    pkg = {};
	struct timeval  endtv = {};
	struct timeval  starttv = {};

	long    sec = 0;
	int     micorsec = 0;

	while (1) {
		byte = zmq_recv(skt, &pkg, sizeof(pkg), 0);
		RAISE_SYS_ERROR(byte);

		if (unlikely(byte > (int)sizeof(pkg))) {
			x_printf(W, "message has been truncate.");
			continue;
		}

		/* ----------------------               */
		/*处理收到的数据*/
		/* ----------------------               */

		gettimeofday(&endtv, NULL);
		starttv.tv_sec = pkg.sec;
		starttv.tv_usec = pkg.microsec;

		sec = endtv.tv_sec - starttv.tv_sec;
		micorsec = endtv.tv_usec - starttv.tv_usec;

		if (micorsec < 0) {
			/* code */
			micorsec += 1000000;
			sec -= 1;
		}

#if 0
		fprintf(stdout,
			LOG_I_COLOR
			"-> %03ld.%06d : %.*s"
			PALETTE_NULL
			"\n"
			, sec, micorsec, byte, pkg.seq);
#else
		x_printf(D, "%03ld.%06d : %.*s", sec, micorsec, byte, pkg.seq);
#endif
	}
}

static void sndrequest(void *skt, struct hostaddr *host)
{
	int flag = 0;

	fprintf(stdout,
		LOG_I_COLOR
		"request for %s:%d"
		PALETTE_NULL
		"\n"
		, host->host, ntohs(host->port));

	flag = zmq_send(skt, host, sizeof(*host), 0);
	RAISE_SYS_ERROR(flag);
}

