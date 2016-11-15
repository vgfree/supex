/**
 * @file   buffer_class.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Mon Oct 21 14:14:01 2012
 *
 * @brief  Header file for buffer class
 *
 *
 */

#ifndef RINOO_MEMORY_BUFFER_CLASS_H_
#define RINOO_MEMORY_BUFFER_CLASS_H_

struct s_buffer;

typedef struct s_buffer_class {
	size_t inisize;
	size_t maxsize;
	int (*init)(struct s_buffer *buffer);
	size_t (*growthsize)(struct s_buffer *buffer, size_t newsize);
	void *(*malloc)(struct s_buffer *buffer, size_t size);
	void *(*realloc)(struct s_buffer *buffer, size_t newsize);
	int (*free)(struct s_buffer *buffer);
} t_buffer_class;

#endif /* !RINOO_MEMORY_BUFFER_CLASS_H_ */
