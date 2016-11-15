/**
 * @file   buffer_iterator.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Sat Feb 15 21:23:15 2014
 *
 * @brief  Buffer iterator header file
 *
 *
 */

#ifndef RINOO_MEMORY_BUFFER_ITERATOR_H_
#define RINOO_MEMORY_BUFFER_ITERATOR_H_

typedef struct s_buffer_iterator {
	size_t offset;
	t_buffer *buffer;
} t_buffer_iterator;

#define BUFFER_ITERATOR_GETTER(name, type)	\
static inline int buffer_iterator_get##name(t_buffer_iterator *iterator, type *value) \
{ \
	if (iterator == NULL) { \
		return -1; \
	} \
	if (iterator->buffer == NULL) {\
		return -1; \
	} \
	if (buffer_ptr(iterator->buffer) == NULL) { \
		return -1; \
	} \
	if (iterator->offset + sizeof(type) > buffer_size(iterator->buffer)) { \
		return -1; \
	} \
	if (value != NULL) { \
		*value = *(type *) (buffer_ptr(iterator->buffer) + iterator->offset); \
	} \
	iterator->offset += sizeof(type); \
	return 0; \
}

static inline void buffer_iterator_set(t_buffer_iterator *iterator, t_buffer *buffer)
{
	iterator->offset = 0;
	iterator->buffer = buffer;
}

static inline int buffer_iterator_position_set(t_buffer_iterator *iterator, size_t pos)
{
	if (pos > buffer_size(iterator->buffer)) {
		return -1;
	}
	iterator->offset = pos;
	return 0;
}

static inline int buffer_iterator_position_inc(t_buffer_iterator *iterator, size_t inc)
{
	if (iterator->offset + inc > buffer_size(iterator->buffer)) {
		return -1;
	}
	iterator->offset += inc;
	return 0;
}

static inline size_t buffer_iterator_position_get(t_buffer_iterator *iterator)
{
	return iterator->offset;
}

static inline void *buffer_iterator_ptr(t_buffer_iterator *iterator)
{
	return buffer_ptr(iterator->buffer) + iterator->offset;
}

static inline bool buffer_iterator_end(t_buffer_iterator *iterator)
{
	return (iterator->offset >= buffer_size(iterator->buffer));
}

BUFFER_ITERATOR_GETTER(short, short)
BUFFER_ITERATOR_GETTER(ushort, unsigned short)
BUFFER_ITERATOR_GETTER(int, int)
BUFFER_ITERATOR_GETTER(uint, unsigned int)
BUFFER_ITERATOR_GETTER(char, char)

static inline int buffer_iterator_gethshort(t_buffer_iterator *iterator, short *value)
{
	if (buffer_iterator_getshort(iterator, value) != 0) {
		return -1;
	}
	*value = ntohs(*value);
	return 0;
}

static inline int buffer_iterator_gethushort(t_buffer_iterator *iterator, unsigned short *value)
{
	if (buffer_iterator_getushort(iterator, value) != 0) {
		return -1;
	}
	*value = ntohs(*value);
	return 0;
}

static inline int buffer_iterator_gethint(t_buffer_iterator *iterator, int *value)
{
	if (buffer_iterator_getint(iterator, value) != 0) {
		return -1;
	}
	*value = ntohl(*value);
	return 0;
}

static inline int buffer_iterator_gethuint(t_buffer_iterator *iterator, unsigned int *value)
{
	if (buffer_iterator_getuint(iterator, value) != 0) {
		return -1;
	}
	*value = ntohl(*value);
	return 0;
}

#endif /* !RINOO_MEMORY_BUFFER_ITERATOR_H_ */
