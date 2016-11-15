/**
 * @file   buffer.c
 * @author Reginald LIPS <reginald.l@gmail.com> - Copyright 2013
 * @date   Thu Dec  3 23:23:17 2009
 *
 * @brief  Contains functions to create a buffer (pointer & size)
 *
 *
 */

#include "rinoo/memory/module.h"

static t_buffer_class default_class = {
	.inisize = RINOO_BUFFER_HELPER_INISIZE,
	.maxsize = RINOO_BUFFER_HELPER_MAXSIZE,
	.init = NULL,
	.growthsize = buffer_helper_growthsize,
	.malloc = buffer_helper_malloc,
	.realloc = buffer_helper_realloc,
	.free = buffer_helper_free,
};

static t_buffer_class static_class = {
	.inisize = 0,
	.maxsize = 0,
	.init = NULL,
	.growthsize = NULL,
	.malloc = NULL,
	.realloc = NULL,
	.free = NULL,
};

/**
 * Creates a new buffer. It uses the buffer class for memory allocation.
 * If class is NULL, then default buffer class is used.
 *
 * @param class Buffer class to be used.
 *
 * @return Pointer to the created buffer.
 */
t_buffer *buffer_create(t_buffer_class *class)
{
	t_buffer *buffer;

	buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	if (class == NULL) {
		class = &default_class;
	}
	buffer->class = class;
	buffer->msize = class->inisize;
	if (class->init != NULL && class->init(buffer) != 0) {
		free(buffer);
		return NULL;
	}
	if (class->malloc != NULL) {
		buffer->ptr = class->malloc(buffer, class->inisize);
		if (buffer->ptr == NULL) {
			free(buffer);
			return NULL;
		}
	}
	return buffer;
}

/**
 * Initializes a static buffer.
 *
 * @param buffer Pointer to the buffer to init.
 * @param ptr Pointer to the static memory.
 * @param size Size of the static memory.
 */
void buffer_static(t_buffer *buffer, void *ptr, size_t size)
{
	buffer->ptr = ptr;
	buffer->size = size;
	buffer->msize = 0;
	buffer->class = &static_class;
}

/**
 * Initializes a buffer to use a specific memory segment.
 * This memory segment needs read & write access.
 * This memory segment won't be extended (reallocated).
 *
 * @param buffer Pointer to the buffer to init.
 * @param ptr Pointer to the memory segment to use.
 * @param msize Size of the memory segment.
 */
void buffer_set(t_buffer *buffer, void *ptr, size_t msize)
{
	buffer->ptr = ptr;
	buffer->size = 0;
	buffer->msize = msize;
	buffer->class = &static_class;
}

/**
 * Destroys a buffer.
 *
 * @param buffer Pointer to the buffer to destroy.
 *
 * @return 0 on success, or -1 if an error occurs.
 */
int buffer_destroy(t_buffer *buffer)
{
	if (buffer->ptr != NULL && buffer->class->free != NULL) {
		if (buffer->class->free(buffer) != 0) {
			return -1;
		}
		buffer->ptr = NULL;
	}
	free(buffer);
	return 0;
}

/**
 * Extends a buffer. It tries to set new size to (size * 2).
 *
 * @param buffer Pointer to the buffer to extend.
 * @param size New desired size.
 *
 * @return 0 if succeeds, -1 if an error occurs.
 */
int buffer_extend(t_buffer *buffer, size_t size)
{
	void *ptr;
	size_t msize;

	if (buffer->class->growthsize == NULL || buffer->class->realloc == NULL) {
		return -1;
	}
	msize = buffer->class->growthsize(buffer, size);
	if (msize < size) {
		return -1;
	}
	ptr = buffer->class->realloc(buffer, msize);
	if (ptr == NULL) {
		return -1;
	}
	buffer->ptr = ptr;
	buffer->msize = msize;
	return 0;
}

/**
 * It is vprintf-like function which tries to add data to the buffer
 * and it will try to extend this buffer if it is to small.
 * This function uses vsnprintf, you can find how to use the format
 * string with man vsnprintf.
 *
 * @param buffer Pointer to the buffer to add data to.
 * @param format Format string which defines subsequent arguments.
 * @param ap Vararg list.
 *
 * @return Number of bytes printed if succeeds, else -1.
 */
int buffer_vprint(t_buffer *buffer, const char *format, va_list ap)
{
	int res;
	va_list ap2;

	va_copy(ap2, ap);
	while (((uint32_t) (res = vsnprintf(buffer->ptr + buffer->size,
				       buffer->msize - buffer->size,
				       format, ap2)) >= buffer->msize - buffer->size) &&
	       buffer_extend(buffer, buffer->size + res + 1) == 0) {
		va_end(ap2);
		va_copy(ap2, ap);
	}
	va_end(ap2);
	buffer->size += res;
	return res;
}

/**
 * It is printf-like function which tries to add data to the buffer
 * and it will try to extend this buffer if it is to small.
 * This function uses buffer_vprint.
 *
 * @param buffer Pointer to the buffer to add data to.
 * @param format Format string which defines subsequent arguments.
 *
 * @return Number of bytes printed if succeeds, else -1.
 */
int buffer_print(t_buffer *buffer, const char *format, ...)
{
	int res;
	va_list ap;

	va_start(ap, format);
	res = buffer_vprint(buffer, format, ap);
	va_end(ap);
	return res;
}

/**
 * Adds data to a buffer. If the buffer is to small, this function
 * will try to extend it.
 *
 * @param buffer Buffer where the data will be added.
 * @param data Data to add to the buffer.
 * @param size Size of data.
 *
 * @return size if data is added to the buffer, else -1.
 */
int buffer_add(t_buffer *buffer, const char *data, size_t size)
{
	if (size + buffer->size > buffer->msize && buffer_extend(buffer, size + buffer->size) < 0) {
		return -1;
	}
	memcpy(buffer->ptr + buffer->size, data, size);
	buffer->size += size;
	return size;
}

/**
 * Adds a string to a buffer. It actually calls buffer_add with strlen
 * of str as size parameter.
 *
 * @param buffer Buffer where the string will be added
 * @param str String to add to the buffer
 *
 * @return Number of bytes added on success, or -1 if an error occurs
 */
int buffer_addstr(t_buffer *buffer, const char *str)
{
	return buffer_add(buffer, str, strlen(str));
}

/**
 * Adds a null byte to the end of a buffer.
 *
 * @param buffer Buffer where the null byte will be added
 *
 * @return 0 on success, or -1 if an error occurs
 */
int buffer_addnull(t_buffer *buffer)
{
	if ((buffer->size == 0 || ((char *) buffer->ptr)[buffer->size - 1] != 0) && buffer_add(buffer, "\0", 1) < 0) {
		return -1;
	}
	return 0;
}

/**
 * Erases beginning data in the buffer and moves the rest
 * to the beginning. This function does -not- reduce the buffer.
 *
 * @param buffer Buffer where data will be erased.
 * @param size Size to erase. If 0, the whole buffer is erased.
 *
 * @return 0 if data has been erased, else -1.
 */
int buffer_erase(t_buffer *buffer, size_t size)
{
	if (buffer->ptr == NULL) {
		return -1;
	}
	if (size == 0 || size >= buffer->size) {
		size = buffer->size;
	} else {
		memmove(buffer->ptr, buffer->ptr + size, buffer->size - size);
	}
	buffer->size -= size;
	return 0;
}

/**
 * Duplicates a buffer.
 *
 * @param buffer Pointer to the buffer to duplicate.
 * @param class Pointer to the class buffer to use in the new buffer.
 *
 * @return A pointer to the new buffer, or NULL if an error occurs.
 */
t_buffer *buffer_dup_class(t_buffer *buffer, t_buffer_class *class)
{
	t_buffer *newbuffer;

	if (class->malloc == NULL) {
		return NULL;
	}
	newbuffer = malloc(sizeof(*newbuffer));
	if (unlikely(newbuffer == NULL)) {
		return NULL;
	}
	*newbuffer = *buffer;
	if (newbuffer->msize == 0) {
		newbuffer->msize = buffer->size;
	}
	newbuffer->class = class;
	if (class->init != NULL && class->init(newbuffer) != 0) {
		free(newbuffer);
		return NULL;
	}
	newbuffer->ptr = class->malloc(newbuffer, newbuffer->msize);
	if (unlikely(newbuffer->ptr == NULL)) {
		free(newbuffer);
		return NULL;
	}
	memcpy(newbuffer->ptr, buffer->ptr, buffer->size);
	return newbuffer;
}

/**
 * Duplicates a buffer.
 *
 * @param buffer Pointer to the buffer to duplicate.
 *
 * @return A pointer to the new buffer, or NULL if an error occurs.
 */
t_buffer *buffer_dup(t_buffer *buffer)
{
	return buffer_dup_class(buffer, buffer->class);
}

/**
 * Compares two buffers
 *
 * @param buffer1 Pointer to a buffer.
 * @param buffer2 Pointer to a buffer.
 *
 * @return An integer less than, equal to, or greater than zero if buffer1 is found, respectively, to be less than, to match, or be greater than buffer2
 */
int buffer_cmp(t_buffer *buffer1, t_buffer *buffer2)
{
	int ret;
	size_t min;

	min = (buffer_size(buffer1) < buffer_size(buffer2) ? buffer_size(buffer1) : buffer_size(buffer2));
	ret = memcmp(buffer_ptr(buffer1), buffer_ptr(buffer2), min);
	if (ret == 0) {
		ret = buffer_size(buffer1) - buffer_size(buffer2);
	}
	return ret;
}

/**
 * Compares a buffer with a string.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than str
 */
int buffer_strcmp(t_buffer *buffer, const char *str)
{
	int ret;
	size_t min;
	size_t len;

	len = strlen(str);
	min = (buffer_size(buffer) < len ? buffer_size(buffer) : len);
	ret = memcmp(buffer_ptr(buffer), str, min);
	if (ret == 0) {
		ret = buffer_size(buffer) - len;
	}
	return ret;
}

/**
 * Compares a buffer to the first n bytes of a string.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 * @param len Maximum length of the string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than s2
 */
int buffer_strncmp(t_buffer *buffer, const char *str, size_t len)
{
	int ret;
	size_t min;

	min = (buffer_size(buffer) < len ? buffer_size(buffer) : len);
	ret = memcmp(buffer_ptr(buffer), str, min);
	if (ret == 0 && buffer_size(buffer) < len) {
		ret = buffer_size(buffer) - len;
	}
	return ret;
}

/**
 * Compares a buffer with a string ignoring case.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than str
 */
int buffer_strcasecmp(t_buffer *buffer, const char *str)
{
	int ret;
	size_t min;
	size_t len;

	len = strlen(str);
	min = (buffer_size(buffer) < len ? buffer_size(buffer) : len);
	ret = strncasecmp(buffer_ptr(buffer), str, min);
	if (ret == 0) {
		ret = buffer_size(buffer) - len;
	}
	return ret;
}

/**
 * Compares a buffer to the first n bytes of a string ignoring case.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 * @param len Maximum length of the string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than s2
 */
int buffer_strncasecmp(t_buffer *buffer, const char *str, size_t len)
{
	int ret;
	size_t min;

	min = (buffer_size(buffer) < len ? buffer_size(buffer) : len);
	ret = strncasecmp(buffer_ptr(buffer), str, min);
	if (ret == 0 && buffer_size(buffer) < len) {
		ret = buffer_size(buffer) - len;
	}
	return ret;
}

/**
 * Converts a buffer to a long int accordingly to strtol.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 * @param base Conversion base.
 *
 * @return Result of conversion.
 */
long int buffer_tolong(t_buffer *buffer, size_t *len, int base)
{
	long int result;
	char *endptr;
	t_buffer *workbuf;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (buffer_addnull(workbuf) == 0) {
		result = strtol(workbuf->ptr, &endptr, base);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer to an unsigned long int accordingly to strtoul.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 * @param base Conversion base.
 *
 * @return Result of conversion.
 */
unsigned long int buffer_toulong(t_buffer *buffer, size_t *len, int base)
{
	char *endptr;
	t_buffer *workbuf;
	unsigned long int result;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (buffer_addnull(workbuf) == 0) {
		result = strtoul(workbuf->ptr, &endptr, base);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer to a float accordingly to strtof.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 *
 * @return Result of conversion.
 */
float buffer_tofloat(t_buffer *buffer, size_t *len)
{
	float result;
	char *endptr;
	t_buffer *workbuf;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (buffer_addnull(workbuf) == 0) {
		result = strtof(workbuf->ptr, &endptr);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer to a double accordingly to strtod.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 *
 * @return Result of conversion.
 */
double buffer_todouble(t_buffer *buffer, size_t *len)
{
	double result;
	char *endptr;
	t_buffer *workbuf;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (buffer_addnull(workbuf) == 0) {
		result = strtod(workbuf->ptr, &endptr);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer into a string.
 * It makes sure the buffer ends with \0 and returns the internal buffer pointer.
 * Buffer length changes, thus.
 *
 * @param buffer Pointer to the buffer to convert.
 *
 * @return A pointer to a string or NULL if an error occurs.
 */
char *buffer_tostr(t_buffer *buffer)
{
	if (buffer_addnull(buffer) != 0) {
		return NULL;
	}
	return buffer->ptr;
}
