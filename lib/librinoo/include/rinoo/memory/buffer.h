/**
 * @file   buffer.h
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue Dec 15 02:20:19 2009
 *
 * @brief  Header file for buffer management
 *
 *
 */

#ifndef RINOO_MEMORY_BUFFER_H_
#define RINOO_MEMORY_BUFFER_H_

#define RINOO_BUFFER_INCREMENT	2048

typedef struct s_buffer {
	void *ptr;
	size_t size;
	size_t msize;
	t_buffer_class *class;
} t_buffer;

#define buffer_ptr(buffer)			((buffer)->ptr)
#define buffer_size(buffer)			((buffer)->size)
#define buffer_msize(buffer)			((buffer)->msize)
#define buffer_isfull(buffer)			((buffer)->size == (buffer)->msize || (buffer)->msize == 0)
#define buffer_setsize(buffer, newsize)		do { (buffer)->size = newsize; } while (0)
#define strtobuffer(buffer, str)		do { buffer_static(buffer, (void *)(str), strlen(str)); } while (0)

t_buffer *buffer_create(t_buffer_class *class);
void buffer_static(t_buffer *buffer, void *ptr, size_t size);
void buffer_set(t_buffer *buffer, void *ptr, size_t msize);
int buffer_destroy(t_buffer *buffer);
int buffer_extend(t_buffer *buffer, size_t size);
int buffer_vprint(t_buffer *buffer, const char *format, va_list ap);
int buffer_print(t_buffer *buffer, const char *format, ...);
int buffer_add(t_buffer *buffer, const char *data, size_t size);
int buffer_addstr(t_buffer *buffer, const char *str);
int buffer_addnull(t_buffer *buf);
int buffer_erase(t_buffer *buffer, size_t size);
t_buffer *buffer_dup(t_buffer *buffer);
int buffer_cmp(t_buffer *buffer1, t_buffer *buffer2);
int buffer_strcmp(t_buffer *buffer, const char *str);
int buffer_strncmp(t_buffer *buffer, const char *str, size_t len);
int buffer_strcasecmp(t_buffer *buffer, const char *str);
int buffer_strncasecmp(t_buffer *buffer, const char *str, size_t len);
long int buffer_tolong(t_buffer *buffer, size_t *len, int base);
unsigned long int buffer_toulong(t_buffer *buffer, size_t *len, int base);
float buffer_tofloat(t_buffer *buffer, size_t *len);
double buffer_todouble(t_buffer *buffer, size_t *len);
char *buffer_tostr(t_buffer *buffer);

#endif /* !RINOO_MEMORY_BUFFER_H_ */
