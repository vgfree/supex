#include "register.h"
#include <assert.h>

#define BUSINESS_MAX_SIZE 30

static business_t g_busi_ary[BUSINESS_MAX_SIZE] = { { 0 } };

int register_business(int busi_type, const business_t *busi)
{
	assert(busi != NULL);

	if ((busi_type < 0) || (busi_type >= BUSINESS_MAX_SIZE)) {
		return -1;
	}

	memcpy(&g_busi_ary[busi_type], busi, sizeof(business_t));

	return 0;
}

int startup_businesses(const sync_conf_t *args)
{
	int i;

	/* g_busi_ary like this "xxx|  |xxx|  |xxx|  ".
	 *   Because evt_type contain { EV_XXX_REQ, EV_XXX_REP, ... }
	 *   BUSI_XXX(s) are only equals to EV_XXX_REP, except EV_DUMP_REQ (it's special).
	 */
	for (i = 0; i < BUSINESS_MAX_SIZE; i++) {
		if (g_busi_ary[i].done != NULL) {
			if (g_busi_ary[i].init(args) == -1) {
				return i;	// Return the busi_type (== i).
			}
		}
	}

	return 0;
}

int do_business(int busi_type, void *args, size_t arg_size)
{
	return (g_busi_ary[busi_type].done != NULL)
	       ? g_busi_ary[busi_type].done(args, arg_size)
	       : -1;
}

void unregister_business(int busi_type)
{
	if (g_busi_ary[busi_type].destroy != NULL) {
		g_busi_ary[busi_type].destroy();
		memset(&g_busi_ary[busi_type], 0, sizeof(business_t));
	}
}

