#include "appsrv.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

int main(int argc, char *argv[])
{
	create_io();

	while (1) {
		struct app_msg  msg = {};
		int             more = 0;
		recv_app_msg(&msg, &more, -1);
		printf("msg size:%d, [0]iov_len:%d\n",
			msg.vector_size, msg.vector[0].iov_len);
		assert(msg.vector_size > 0);

		if (memcmp(msg.vector[0].iov_base, "status", 6) == 0) {
			if (memcmp(msg.vector[1].iov_base, "connected", 9) == 0) {
				char            setting[] = "setting";
				char            gidmap[] = "gidmap";
				char            cid[20] = {}; memcpy(cid, msg.vector[2].iov_base, msg.vector[2].iov_len);
				char            gid[] = "gid0";
				struct app_msg  send_msg = {};
				send_msg.vector_size = 4;
				send_msg.vector[0].iov_base = setting;
				send_msg.vector[0].iov_len = strlen(setting);
				send_msg.vector[1].iov_base = gidmap;
				send_msg.vector[1].iov_len = strlen(gidmap);
				send_msg.vector[2].iov_base = cid;
				send_msg.vector[2].iov_len = strlen(cid);
				send_msg.vector[3].iov_base = gid;
				send_msg.vector[3].iov_len = strlen(gid);
				send_app_msg(&send_msg);
			}
		} else {
			struct app_msg send_msg = {};
			send_msg.vector_size = 3 + msg.vector_size;
			char    downstream[] = "downstream";
			char    gid[] = "gid";
			char    gid0[] = "gid0";
			send_msg.vector[0].iov_base = downstream;
			send_msg.vector[0].iov_len = strlen(downstream);
			send_msg.vector[1].iov_base = gid;
			send_msg.vector[1].iov_len = strlen(gid);
			send_msg.vector[2].iov_base = gid0;
			send_msg.vector[2].iov_len = strlen(gid0);

			for (size_t i = 0; i < msg.vector_size; i++) {
				send_msg.vector[i + 3].iov_base = msg.vector[i].iov_base;
				send_msg.vector[i + 3].iov_len = msg.vector[i].iov_len;
			}

			send_app_msg(&send_msg);
		}

		for (size_t i = 0; i < msg.vector_size; i++) {
			free(msg.vector[i].iov_base);
		}
	}

	destroy_io();
}

