/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_structure.h"

inline bool new_commmsg(struct comm_message** message, int size)
{
	assert(message);

	New(*message);
	if (*message) {
		NewArray((*message)->content, size);
		if ((*message)->content) {
			return true;
		} else {
			Free(*message);
		}
	}
	return false;
}

inline void copy_commmsg(struct comm_message* destmsg, const struct comm_message* srcmsg)
{
	assert(destmsg && srcmsg && destmsg->content && srcmsg->content);
	destmsg->fd = srcmsg->fd;
	destmsg->config = srcmsg->config;
	destmsg->socket_type = srcmsg->socket_type;
	memcpy(&destmsg->package, &srcmsg->package, sizeof(srcmsg->package));
	memcpy(destmsg->content, srcmsg->content, srcmsg->package.dsize);
}

inline void free_commmsg(void* arg)
{
	struct comm_message *message = (struct comm_message*)arg;
	if (message) {
		Free(message->content);
		Free(message);
	}
}
