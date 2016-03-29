/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file   kv_public.h
 * @brief  Shared variants stores in global static area used by all the instances.
 *
 * @author shishengjie
 * @date   2015-07-03
 */

#include <pthread.h>

/** error table */
//const char* err_table_get(int errnum);


/** locks */

/**
 * Fast lock for keeping short time lock.
 */
#define FAST_LOCK_ON  pthread_spin_lock(&g_fast_lock)
#define FAST_LOCK_TRY pthread_spin_trylock(&g_fast_lock)
#define FAST_LOCK_OFF pthread_spin_unlock(&g_fast_lock)

/**
 * Mutex lock for critical resources.
 */
#define MUTEX_LOCK_ON   pthread_mutex_lock(&g_mutex_lock)
#define MUTEX_LOCK_TRY  pthread_mutex_trylock(&g_mutex_lock)
#define MUTEX_LOCK_OFF  pthread_mutex_unlock(&g_mutex_lock)





/** lock interface */
#define LOCK(lock_type)     lock_type##_ON
#define LOCK_TRY(lock_type) lock_type##_TRY
#define UNLOCK(lock_type)   lock_type##_OFF


