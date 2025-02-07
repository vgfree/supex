#ifndef _REGISTER_H_
#define _REGISTER_H_

#include "netmod.h"
#include "conf.h"

/* Business type defined. */
#define BUSI_DUMP       NET_EV_DUMP_REQ
#define BUSI_INCR       NET_EV_INCREMENT_REP

/* Error types. */
#define BUSI_ERR_FATAL  NET_EV_FATAL	// Serious error, Client will exit.
#define BUSI_ERR_FAIL   NET_EV_FAIL	// Common error, exit or not please see protocol.

struct business_t
{
	int     (*init)(struct pole_conf *conf);
	int     (*done)(void *args, size_t arg_size);
	void    (*destroy)();
};
typedef struct business_t business_t;

/*************************************************************************
 * FUNCTION:
 *   register_business().
 * DESCRIPTION:
 *   Register business with business type [BUSI_DUMP|BUSI_EXEC_SQL|...];
 * INPUT:
 *   busi_type:
 *     The business type, [BUSI_DUMP|BUSI_EXEC_SQL|...].
 *   busi:
 *     The business struct pointer.
 *
 * RETURN:
 *   SUCC: 0.
 *   FAIL: -1, register queue already full, or busi_type is invalid.
 * OTHERS:
 *   (null)
 ************************************************************************/
int register_business(int busi_type, const business_t *busi);

/*************************************************************************
 * FUNCTION:
 *   register_business().
 * DESCRIPTION:
 *   Startup all the businesses, with parameter args;
 * INPUT:
 *   args:
 *     The business function(business_t.init(void *))'s parameter.
 *
 * RETURN:
 *   SUCC: 0.
 *   FAIL: Return the business type.
 * OTHERS:
 *   (null)
 ************************************************************************/
int startup_businesses(struct pole_conf *args);

/*************************************************************************
 * FUNCTION:
 *   do_business().
 * DESCRIPTION:
 *   Doing business with business type [BUSI_DUMP|BUSI_EXEC_SQL|...] and
 *     parameter args.
 * INPUT:
 *   busi_type:
 *     The business type, [BUSI_DUMP|BUSI_EXEC_SQL|...].
 *   args:
 *     The business function(business_t.do(int, void *))'s parameter.
 *   arg_size:
 *     The argument size.
 *
 * RETURN:
 *   SUCC: 0.
 *   FAIL: if busi_type doesn't exist, return -1, else return BUSI_ERR_XX.
 * OTHERS:
 *   (null)
 ************************************************************************/
int do_business(int busi_type, void *args, size_t arg_size);

/*************************************************************************
 * FUNCTION:
 *   unregister_business().
 * DESCRIPTION:
 *   Unregister business with business type [BUSI_DUMP|BUSI_EXEC_SQL|...]
 * INPUT:
 *   busi_type:
 *     The business type, [BUSI_DUMP|BUSI_EXEC_SQL|...].
 *
 * RETURN:
 *   SUCC: 0.
 *   FAIL: -1, register queue already full, or busi_type is invalid.
 * OTHERS:
 *   (null)
 ************************************************************************/
void unregister_businesses(int busi_type);
#endif	/* ifndef _REGISTER_H_ */

