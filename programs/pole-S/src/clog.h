/*
 * LOG Module, common usage.
 *
 * Create By: liubaoan@mirrtalk.com
 *      Date: 2015/09/23
 * WeChat ID: lbajean
 *
 */

#ifndef _CLOG_H_
#define _CLOG_H_

#include "slog/slog.h"

/* Log level */
#define LOG_DEBUG       0
#define LOG_INFO        1
#define LOG_WARN        2
#define LOG_ERROR       3
#define LOG_SYS         4
#define LOG_FATAL       5

int log_ctx_init(const char *log_path, int log_level);
#endif	/* _CLOG_H_ */

