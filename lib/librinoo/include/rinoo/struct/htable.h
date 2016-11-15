/**
 * @file   htable.h
 * @author reginaldl <reginald.@gmail.com> - Copyright 2013
 * @date   Tue Jul 29 20:53:47 2014
 *
 * @brief  Hash table structure
 *
 *
 */

#ifndef RINOO_STRUCT_HTABLE_H_
#define RINOO_STRUCT_HTABLE_H_

typedef struct s_htable_node {
	uint32_t hash;
	struct s_htable_node *prev;
	struct s_htable_node *next;
} t_htable_node;

typedef struct s_htable {
	size_t size;
	size_t table_size;
	t_htable_node **table;
	uint32_t (*hash)(t_htable_node *node);
	int (*compare)(t_htable_node *node1, t_htable_node *node2);
} t_htable;

int htable(t_htable *htable, size_t size, uint32_t (*hash)(t_htable_node *node), int (*compare)(t_htable_node *node1, t_htable_node *node2));
void htable_destroy(t_htable *htable);
void htable_flush(t_htable *htable, void (*delete)(t_htable_node *node1));
size_t htable_size(t_htable *htable);
void htable_put(t_htable *htable, t_htable_node *node);
t_htable_node *htable_get(t_htable *htable, t_htable_node *node);
int htable_remove(t_htable *htable, t_htable_node *node);

#endif /* !RINOO_STRUCT_HTABLE_H_ */
