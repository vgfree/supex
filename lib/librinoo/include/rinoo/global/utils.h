/**
 * @file   utils.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Mar 20 18:13:53 2012
 *
 * @brief  Header file for global utils
 *
 *
 */

#ifndef RINOO_UTILS_H_
#define RINOO_UTILS_H_

#define ARRAY_SIZE(array)	(sizeof(array) / sizeof(array[0]))

#define RINOO_LOG_MAXLENGTH	2048

void rinoo_log(const char *format, ...);

#endif /* !RINOO_UTILS_H_ */
