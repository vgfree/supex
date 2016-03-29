#ifndef __JTT_CLIENT_H__
#define __JTT_CLIENT_H__

#include "pub_incl.h"

int is_ok_connect(int sockfd);

// int jtt_connect(const char *ip,short port,time_t *tm);
int jtt_connect(const char *ip, short port, int *sockfd);

int jtt_disconnect(int sockfd);

int jtt_recv(int sockfd, unsigned char *buf, int *len, struct timeval tv);

int jtt_send(int sockfd, unsigned char *buf, int len, struct timeval tv);
#endif

