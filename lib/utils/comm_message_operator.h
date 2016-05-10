#ifndef _COMM_MESSAGE_OPERATOR_H_
#define _COMM_MESSAGE_OPERATOR_H_
#include "comm_structure.h"

int get_msg_fd(struct comm_message *msg);
void set_msg_fd(struct comm_message *msg, int fd);
char *get_msg_frame(int index, struct comm_message *msg, int *size);
int set_msg_frame(int index, struct comm_message *msg, int size, char *frame);
#endif
