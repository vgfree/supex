#pragma once

static inline void commmsg_print(struct comm_message *message)
{
	int i = 0;
	for (; i < commmsg_frame_count(message); i++) {
		int     fsize = 0;
		char    *frame = commmsg_frame_get(message, i, &fsize);

		char    *tmp = malloc(sizeof(char) * (fsize + 1));
		memcpy(tmp, frame, fsize);
		tmp[fsize] = '\0';
		x_printf(D, "%d frame, data:%s", i, tmp);
		free(tmp);
	}
	x_printf(D, "message type:%d.", message->ptype);
}
