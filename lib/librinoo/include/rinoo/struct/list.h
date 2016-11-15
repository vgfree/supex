/**
 * @file   list.h
 * @author reginaldl <reginald.@gmail.com> - Copyright 2013
 * @date   Sun Mar 24 23:18:12 2013
 *
 * @brief  LIST structure
 *
 *
 */

#ifndef RINOO_STRUCT_LIST_H_
#define RINOO_STRUCT_LIST_H_

typedef struct s_list_node {
	struct s_list_node *prev;
	struct s_list_node *next;
} t_list_node;

typedef struct s_list {
	size_t size;
	t_list_node *head;
	t_list_node *tail;
	int (*compare)(t_list_node *node1, t_list_node *node2);
} t_list;

int list(t_list *list, int (*compare)(t_list_node *node1, t_list_node *node2));
void list_flush(t_list *list, void (*delete)(t_list_node *node1));
size_t list_size(t_list *list);
void list_put(t_list *list, t_list_node *node);
t_list_node *list_get(t_list *list, t_list_node *node);
int list_remove(t_list *list, t_list_node *node);
t_list_node *list_pop(t_list *list);
t_list_node *list_head(t_list *list);

#endif /* !RINOO_STRUCT_LIST_H_ */
