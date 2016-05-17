#ifndef _STATUS_H_
#define _STATUS_H_

int erase_client(int fd);
void send_status_msg(int clientfd, int status);
#endif
