/**
 * @file   list.c
 * @author reginaldl <reginald.@gmail.com> - Copyright 2013
 * @date   Sun Mar 24 23:21:28 2013
 *
 * @brief  LIST functions
 *
 *
 */

#include "rinoo/struct/module.h"

/**
 * Initializes a list.
 * If a compare function is provided, the list will be sorted.
 *
 * @param list List to initialize
 * @param compare Optional compare function
 *
 * @return 0 on success otherwise -1
 */
int list(t_list *list, int (*compare)(t_list_node *node1, t_list_node *node2))
{
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->compare = compare;
	return 0;
}

/**
 * Flushes list content.
 *
 * @param list List to flush
 * @param delete Optional delete function to be called for each list node
 */
void list_flush(t_list *list, void (*delete)(t_list_node *node1))
{
	t_list_node *node;

	if (delete != NULL) {
		while ((node = list_pop(list)) != NULL) {
			delete(node);
		}
	}
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

/**
 * Gets list size.
 *
 * @param list List to use
 *
 * @return List size
 */
size_t list_size(t_list *list)
{
	return list->size;
}

/**
 * Adds an element to a list
 *
 * @param list Pointer to the list
 * @param node Pointer to the node to add
 */
void list_put(t_list *list, t_list_node *node)
{
	node->prev = NULL;
	node->next = list->head;
	if (list->compare != NULL) {
		while (node->next != NULL && list->compare(node->next, node) < 0) {
			node->prev = node->next;
			node->next = node->next->next;
		}
	}
	if (node->prev == NULL) {
		list->head = node;
	} else {
		node->prev->next = node;
	}
	if (node->next == NULL) {
		list->tail = node;
	} else {
		node->next->prev = node;
	}
	list->size++;
}

/**
 * Gets a node from a list.
 *
 * @param list List to use
 * @param node Dummy node used for comparison
 *
 * @return Pointer to the matching node or NULL if not found.
 */
t_list_node *list_get(t_list *list, t_list_node *node)
{
	if (list->compare == NULL) {
		return NULL;
	}
	node->next = list->head;
	while (node->next != NULL) {
		if (list->compare(node->next, node) == 0) {
			return node->next;
		}
		node->next = node->next->next;
	}
	return NULL;
}

/**
 * Removes an element from a list
 *
 * @param list Pointer to the list
 * @param node Pointer to the element to remove
 *
 * @return 0 on success otherwise -1 if an error occurs
 */
int list_remove(t_list *list, t_list_node *node)
{
	if (node->prev == NULL && node->next == NULL && list->head != node) {
		/* Node already removed */
		return -1;
	}
	if (node->prev != NULL) {
		node->prev->next = node->next;
	} else {
		list->head = node->next;
	}
	if (node->next != NULL) {
		node->next->prev = node->prev;
	} else {
		list->tail = node->prev;
	}
	list->size--;
	node->prev = NULL;
	node->next = NULL;
	return 0;
}

/**
 * Pops an element from a list
 *
 * @param list Pointer to the list
 *
 * @return First element in the list or NULL if empty
 */
t_list_node *list_pop(t_list *list)
{
	t_list_node *node;

	node = list->head;
	if (node != NULL) {
		list_remove(list, node);
	}
	return node;
}

/**
 * Returns list's head
 *
 * @param list Pointer to the list
 *
 * @return First element in the list or NULL if empty
 */
t_list_node *list_head(t_list *list)
{
	return list->head;
}
