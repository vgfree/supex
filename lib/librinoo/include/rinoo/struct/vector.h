/**
 * @file   vector.h
 * @author Reginald Lips <reginald.@gmail.com> - Copyright 2014
 * @date   Wed Apr 30 18:48:12 2014
 *
 * @brief  Vector structure
 *
 *
 */

#ifndef RINOO_STRUCT_VECTOR_H_
#define RINOO_STRUCT_VECTOR_H_

typedef struct s_vector {
	size_t size;
	size_t msize;
	void **ptr;
} t_vector;

int vector_add(t_vector *vector, void *item);
void vector_destroy(t_vector *vector);
int vector_remove(t_vector *vector, uint32_t i);
void *vector_get(t_vector *vector, uint32_t i);
size_t vector_size(t_vector *vector);

#endif /* !RINOO_STRUCT_VECTOR_H_ */

