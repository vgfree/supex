//
//  sample1.c
//  libmini
//
//  Created by 周凯 on 15/10/31.
//  Copyright © 2015年 zk. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "http_parser.h"

static int message_begin(http_parser *parser);

static int url(http_parser *parser, const char *at, size_t length);

static int status(http_parser *parser, const char *at, size_t length);

static int header_field(http_parser *parser, const char *at, size_t length);

static int header_value(http_parser *parser, const char *at, size_t length);

static int header_complete(http_parser *parser);

static int body(http_parser *parser, const char *at, size_t length);

static int message_complete(http_parser *parser);

const static http_parser_settings settings = {
	.on_message_begin       = message_begin,
	.on_url                 = url,
	.on_status              = status,
	.on_header_field        = header_field,
	.on_header_value        = header_value,
	.on_headers_complete    = header_complete,
	.on_body                = body,
	.on_message_complete    = message_complete,
};

const char post_http_header[] =
	"POST /data HTTP/1.0\r\n"
	"User-Agent: Wget/1.11.4\r\n"
	"Accept: */*\r\n"
	"Content-Length: 3\r\n"
	"Host: www.163.com\r\n"
	"Connection: Keep-Alive\r\n\r\n";

const char post_http_body[] = "abc";

const char get_http_header[] =
	"GET /index.html HTTP/1.0\r\n"
	"User-Agent: Wget/1.11.4\r\n"
	"Accept: */*\r\n"
	"Host: www.163.com\r\n"
	"Connection: Keep-Alive\r\n\r\n";

const char response_http[] =
	"HTTP/1.0 200 OK\r\n"
	"User-Agent: Wget/1.11.4\r\n"
	"Content-Length: 3\r\n"
	"Connection: Keep-Alive\r\n\r\n"
	"ABC";

int main(int argc, char **argv)
{
	size_t          todo = 0;
	http_parser     parser = {};

	printf("------------ parse request start ------------ \n");

	http_parser_init(&parser, HTTP_REQUEST);

	printf("---------- parse header ------------ \n");

	todo = http_parser_execute(&parser, &settings, post_http_header, sizeof(post_http_header) - 1);
	assert(todo == sizeof(post_http_header) - 1);
#if 0
	/*不能再次解析已解析过的数据*/
	todo = http_parser_execute(&parser, &settings, post_http_header, sizeof(post_http_header) - 1);
	assert(todo == sizeof(post_http_header) - 1);
#endif

	printf("---------- parse body : %lld   ------------ \n", parser.content_length);

	todo = http_parser_execute(&parser, &settings, post_http_body, sizeof(post_http_body) - 1);
	assert(todo == sizeof(post_http_body) - 1);

	printf("------------ parse request end   ------------ \n");

	return EXIT_SUCCESS;
}

static int message_begin(http_parser *parser)
{
	printf("call %s\n", __FUNCTION__);
	return 0;
}

static int url(http_parser *parser, const char *at, size_t length)
{
	printf("call %s:%.*s\n", __FUNCTION__, (int)length, at);
	return 0;
}

static int status(http_parser *parser, const char *at, size_t length)
{
	printf("call %s:%.*s\n", __FUNCTION__, (int)length, at);
	return 0;
}

static int header_field(http_parser *parser, const char *at, size_t length)
{
	printf("call %s:%.*s\n", __FUNCTION__, (int)length, at);
	return 0;
}

static int header_value(http_parser *parser, const char *at, size_t length)
{
	printf("call %s:%.*s\n", __FUNCTION__, (int)length, at);
	return 0;
}

static int header_complete(http_parser *parser)
{
	printf("call %s\n", __FUNCTION__);
	return 0;
}

static int body(http_parser *parser, const char *at, size_t length)
{
	printf("call %s:%.*s\n", __FUNCTION__, (int)length, at);
	return 0;
}

static int message_complete(http_parser *parser)
{
	printf("call %s\n", __FUNCTION__);
	return 0;
}

