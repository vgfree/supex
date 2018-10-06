#include <stdio.h>
#include <string.h>
#include "tpl.h"

const char *filename = "/tmp/test114.tpl";
extern tpl_hook_t tpl_hook;

struct msg {
	int a;
	int b;
	char c[8];
	uint64_t d;
};

static int __fcb(void *img, size_t sz, void *data)
{
        char *fmt = tpl_peek(TPL_MEM, img, sz);
	printf("%s\n", fmt);

        struct msg                   msg;
        tpl_node *tn = tpl_map(fmt, &msg, 8);
        tpl_load(tn, TPL_MEM, img, sz);
        tpl_unpack(tn, 0);

	printf("%d %d %c %c %c %c %c %c %c %c %ld\n",
			msg.a, msg.b, msg.c[0], msg.c[1], msg.c[2], msg.c[3], msg.c[4], msg.c[5], msg.c[6], msg.c[7], msg.d);
}


int main() {
	char g_buf[512] = {0};
	size_t g_len = 0;
	struct msg msg = {0};
	msg.a = 1;
	msg.b = 2;
#if 1
	msg.c[0] = 'a';
	msg.c[1] = 'b';
	msg.c[2] = 'c';
	msg.c[3] = 'd';
	msg.c[4] = 'e';
	msg.c[5] = 'f';
	msg.c[6] = 'g';
	msg.c[7] = '\0';
#endif
	msg.d = 3;

	tpl_hook.oops = printf;

	tpl_node *tn;

	tn = tpl_map("S(iic#I)", &msg, 8);
	if (!tn) {
		printf("tpl_map failed; exiting\n");
		return -1;
	}
	tpl_pack(tn,0);
	tpl_dump(tn, TPL_GETSIZE, &g_len);
        tpl_dump(tn, TPL_MEM | TPL_PREALLOCD, g_buf, 512);
	tpl_dump(tn,TPL_FILE,filename);
	tpl_free(tn);

	memset(&msg, 0, sizeof(msg));


	tn = tpl_map("S(iic#I)", &msg, 8);
	if (!tn) {
		printf("tpl_map failed; exiting\n");
		return -1;
	}
	tpl_load(tn,TPL_FILE,filename);
	tpl_unpack(tn,0);
	tpl_free(tn);

	printf("%d %d %c %c %c %c %c %c %c %c %ld\n",
			msg.a, msg.b, msg.c[0], msg.c[1], msg.c[2], msg.c[3], msg.c[4], msg.c[5], msg.c[6], msg.c[7], msg.d);



	tpl_gather_t            *gt = NULL;
	tpl_gather(TPL_GATHER_MEM, g_buf, g_len, &gt, __fcb, NULL);
	return 0;
}
