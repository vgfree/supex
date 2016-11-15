/**
 * @file   buffer_helper.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Oct 21 14:18:35 2012
 *
 * @brief  Header for buffer helper
 *
 *
 */

#ifndef RINOO_MEMORY_BUFFER_HELPER_H_
#define RINOO_MEMORY_BUFFER_HELPER_H_

#define RINOO_BUFFER_HELPER_INISIZE	1024
#define RINOO_BUFFER_HELPER_MAXSIZE	(1024 * 1024 * 1024)

size_t buffer_helper_growthsize(t_buffer *buffer, size_t newsize);
void *buffer_helper_malloc(t_buffer *buffer, size_t size);
void *buffer_helper_realloc(t_buffer *buffer, size_t newsize);
int buffer_helper_free(t_buffer *buffer);

#endif /* !RINOO_MEMORY_BUFFER_HELPER_H_ */
