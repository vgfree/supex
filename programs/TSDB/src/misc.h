#pragma once

#include <stddef.h>

// #define GET_NEED_COUNT(idx, div)        ( (idx) / (div) + ( (idx) % (div) >= 1 )? 1 : 0 )  --> it's error, this result allway is 0 or 1
#define GET_NEED_COUNT(idx, div)        ((idx) / (div) + (((idx) % (div) >= 1) ? 1 : 0))

#ifndef MIN
  #define MIN(x, y)                     (((x) <= (y)) ? (x) : (y))
#endif
#ifndef MAX
  #define MAX(x, y)                     (((x) >= (y)) ? (x) : (y))
#endif

#ifdef USE_MUTEX
  #define X_NEW_LOCK pthread_mutex_t
  #define X_LOCK_INIT(lock)     pthread_mutex_init((lock), NULL)
  #define X_LOCK(lock)          pthread_mutex_lock((lock))
  #define X_UNLOCK(lock)        pthread_mutex_unlock((lock))
#else
  #define X_NEW_LOCK pthread_spinlock_t
  #define X_LOCK_INIT(lock)     pthread_spin_init((lock), 0)
  #define X_LOCK(lock)          pthread_spin_lock((lock))
  #define X_UNLOCK(lock)        pthread_spin_unlock((lock))
#endif

#define ONE_DAY_TIMESTAMP (24 * 60 * 60)

extern void check_pid_file(void);

extern void write_pid_file(void);

extern void remove_pid_file(void);

char *x_strdup(const char *src);

int get_overplus_time(void);

int get_current_time(void);

/*
 * Regular string match for specified length.
 */
int string_match_len(const char *pattern, int patternLen, const char *string, int stringLen, int nocase);

/*
 * Regular string match.
 */
int string_match(const char *pattern, const char *string, int nocase);

/*
 * Prefix match.
 */
int prefix_match_len(const char *pre, int preLen, const char *string, int stringLen);

int get_number_len(size_t len);

char *set_bulk(char *dst, const char *put, int len);

