#ifndef _COMM_MESSAGE_OPERATOR_H_
#define _COMM_MESSAGE_OPERATOR_H_
#include "comm_structure.h"

void init_msg(struct comm_message *msg);
void destroy_msg(struct comm_message *msg);
int get_msg_fd(struct comm_message *msg);
void set_msg_fd(struct comm_message *msg, int fd);
char *get_msg_frame(int index, struct comm_message *msg, int *size);
int set_msg_frame(int index, struct comm_message *msg, int size, char *frame);
int remove_first_nframe(int nframe, struct comm_message *msg);
int get_max_msg_frame(struct comm_message *msg);
int get_frame_size(int index, struct comm_message *msg);
#endif
