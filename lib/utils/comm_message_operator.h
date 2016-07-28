#ifndef _COMM_MESSAGE_OPERATOR_H_
#define _COMM_MESSAGE_OPERATOR_H_
#include "comm_structure.h"

struct residue_package
{
	int     fd;
	int     offset;
	char    *serial_data;
	int     serial_size;
};

inline int init_residue_package(struct residue_package *package, int fd, int offset, char *_serial_data, int _serial_size)
{
	package->fd = fd;
	package->offset = offset;
	package->serial_data = _serial_data;
	package->serial_size = _serial_size;
}

inline int push_residue_package(struct residue_package *package)
{
	// to do;
	return 0;
}

inline int pop_residue_package(struct residue_package *package)
{
	// to do:
	return 0;
}

inline void destroy_residue_package(struct residue_package *package)
{
	if (package->serial_data) {
		free(package->serial_data);
	}
}

void init_msg(struct comm_message *msg);

void destroy_msg(struct comm_message *msg);

int get_msg_fd(struct comm_message *msg);

void set_msg_fd(struct comm_message *msg, int fd);

char *get_msg_frame(int index, struct comm_message *msg, int *size);

int set_msg_frame(int index, struct comm_message *msg, int size, char *frame);

int remove_first_nframe(int nframe, struct comm_message *msg);

int get_max_msg_frame(struct comm_message *msg);

int get_frame_size(int index, struct comm_message *msg);

#endif	/* ifndef _COMM_MESSAGE_OPERATOR_H_ */

