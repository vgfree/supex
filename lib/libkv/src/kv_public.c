/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file   kv_public.c
 * @brief  Shared variants stores in global static area used by all the instances.
 *
 * @author shishengjie
 * @date   2015-07-03
 */

#include "kv_inner.h"



/**
 * @ingroup group_error_code
 * @{
 */

#define ERR_ITEM_ADD(index,msg) msg
char *err_table[] = {
        ERR_ITEM_ADD(ERR_NONE, "no error"),
        ERR_ITEM_ADD(ERR_TYPE, "-ERR wrong type"),
        ERR_ITEM_ADD(ERR_NO_KEY, "-ERR no such key"),
        ERR_ITEM_ADD(ERR_SYNTAX, "-ERR syntax error"),
        ERR_ITEM_ADD(ERR_SAME_OBJECT, "-ERR source and destination objects are the same"),
        ERR_ITEM_ADD(ERR_OUT_OF_RANGE, "-ERR index out of range"),
        ERR_ITEM_ADD(ERR_NIL, "-ERR nil"),
        ERR_ITEM_ADD(ERR_NOT_INIT, "-ERR not init"),
        ERR_ITEM_ADD(ERR_BUSY, "-ERR busy"),
        ERR_ITEM_ADD(ERR_ARGUMENTS, "-ERR wrong number of arguments"),
        ERR_ITEM_ADD(ERR_PROTOCOL, "-ERR protocol error"),
        ERR_ITEM_ADD(ERR_VALUE, "-ERR value invalid"),
        ERR_ITEM_ADD(ERR_DB_INDEX, "-ERR invalid DB index"),
        ERR_ITEM_ADD(ERR_CMD, "-ERR unknown command")
};

inline const char* get_err(int errnum)
{
        return err_table[errnum];
}

/**
 * @}
 */



/** default 0->pthread_spin_init */
pthread_spinlock_t g_fast_lock;
pthread_mutex_t g_mutex_lock = PTHREAD_MUTEX_INITIALIZER;

