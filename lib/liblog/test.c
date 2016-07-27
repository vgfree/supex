#include "loger.h"

#define MODULE_NAME                     "CoreExchangeNode"
struct CSLog *g_imlog = NULL;

void main(void)
{
	g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
	log("MGcliIP:%s, MGcliPort:%s.", "xxx", "yyy");
	CSLog_destroy(g_imlog);
}
