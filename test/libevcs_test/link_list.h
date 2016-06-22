/* @author: QianYe(coordcn@163.com)
 * @overview: doubly linked list from linux kernel
 * @reference: stddef.h offsetof
 *             kernel.h container_of
 *             poison.h LIST_POISON1 LIST_POISON2
 */

#ifndef LINK_LIST_H
#define LINK_LIST_H

#include "stddef.h"

#ifndef offset_of
#define offset_of(type, member) (size_t)&(((type*)0)->member)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({ \
  const typeof( ((type *)0)->member ) *__mptr = (ptr); \
  (type *)( (char *)__mptr - offset_of(type,member) );})
#endif

/**
  Simple doubly linked list implementation.
  Some of the internal functions ("__xxx") are useful when
  manipulating whole lists rather than single entries, as
  sometimes we already know the next/prev entries and we can
  generate better code by using them directly rather than
  using the generic single-entry routines.
 */

struct link_list_s {
  struct link_list_s *next;
  struct link_list_s *prev;
};

typedef struct link_list_s link_list_t;

static inline void link_list_init(link_list_t *list) {
  list->next = list;
  list->prev = list;
}

static inline void link_list__insert(link_list_t *new, 
                                      link_list_t *prev, 
                                      link_list_t *next) {
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}

static inline void link_list_insert_head(link_list_t *new,
                                          link_list_t *head) {
  link_list__insert(new, head, head->next);
}

static inline void link_list_insert_tail(link_list_t *new, 
                                          link_list_t *head) {
  link_list__insert(new, head->prev, head);
}

static inline void link_list__remove(link_list_t *prev, 
                                      link_list_t *next) {
  prev->next = next;
  next->prev = prev;
}

static inline void link_list_remove(link_list_t *entry) {
  link_list__remove(entry->prev, entry->next);
}

static inline void link_list_remove_init(link_list_t *entry) {
  link_list__remove(entry->prev, entry->next);
  link_list_init(entry);
}

static inline void link_list_move_head(link_list_t *list, 
                                        link_list_t *head) {
  link_list__remove(list->prev, list->next);
  link_list_insert_head(list, head);
}

static inline void link_list_move_tail(link_list_t *list, 
                                        link_list_t *head) {
  link_list__remove(list->prev, list->next);
  link_list_insert_tail(list, head);
}

static inline int link_list_is_empty(const link_list_t *head) {
  return head->next == head;
}

static inline int link_list_is_single(const link_list_t *head) {
  return !link_list_is_empty(head) && (head->next == head->prev);
}

static inline int link_list_is_last(const link_list_t *list, 
                                     const link_list_t *head) {
  return list->next == head;
}

/**
  link_list_entry - get the struct for this entry
  @ptr:	the &link_list_t pointer.
  @type: the type of the struct this is embedded in.
  @member:	the name of the link_list_t within the struct.
 */
#define link_list_entry(ptr, type, member) \
  container_of(ptr, type, member)

/**
  link_list_first_entry - get the first element from a list
  @ptr:	the list head to take the element from.
  @type: the type of the struct this is embedded in.
  @member:	the name of the link_list_t within the struct.
  Note that if the list is empty, it returns NULL.
 */
#define link_list_first_entry(ptr, type, member) \
  (!link_list_is_empty(ptr) ? link_list_entry((ptr)->next, type, member) : NULL)

/**
  link_list_next_entry - get the next element in list
  @pos:	the type * to cursor
  @member:	the name of the link_list_t within the struct.
 */
#define link_list_next_entry(pos, member) \
  link_list_entry((pos)->member.next, typeof(*(pos)), member)

/**
  link_list_prev_entry - get the prev element in list
  @pos:	the type * to cursor
  @member:	the name of the link_list_t within the struct.
 */
#define link_list_prev_entry(pos, member) \
  link_list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
  link_list_foreach	-	iterate over a list
  @pos:	the &link_list_t to use as a loop cursor.
  @head:	the head for your list.
 */
#define link_list_foreach(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

/**
  link_list_foreach_reverse	-	iterate over a list backwards
  @pos:	the &link_list_t to use as a loop cursor.
  @head:	the head for your list.
 */
#define link_list_foreach_reverse(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
  link_list_foreach_entry	-	iterate over list of given type
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the link_list_t within the struct.
 */
#define link_list_foreach_entry(pos, head, member)				\
  for (pos = link_list_entry((head)->next, typeof(*pos), member);	\
       &pos->member != (head); 	\
       pos = link_list_entry(pos->member.next, typeof(*pos), member))

/**
  link_list_foreach_entry_reverse - iterate backwards over list of given type.
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the list_struct within the struct.
 */
#define link_list_foreach_entry_reverse(pos, head, member)			\
  for (pos = link_list_entry((head)->prev, typeof(*pos), member);	\
       &pos->member != (head); 	\
       pos = link_list_entry(pos->member.prev, typeof(*pos), member))

/**
  Double linked lists with a single pointer list head.
  Mostly useful for hash tables where the two pointer list head is
  too wasteful.
  You lose the ability to access the tail in O(1).
 */

struct link_hlist_node_s {
  struct link_hlist_node_s *next;
  struct link_hlist_node_s **pprev;
};

struct link_hlist_head_s {
  struct link_hlist_node_s *first;
};

typedef struct link_hlist_node_s link_hlist_node_t;
typedef struct link_hlist_head_s link_hlist_head_t;

#define link_hlist_head_init(ptr) ((ptr)->first = NULL)

static inline void link_hlist_node_init(link_hlist_node_t *node) {
  node->next = NULL;
  node->pprev = NULL;
}

static inline int link_hlist_is_empty(link_hlist_head_t *head) {
  return !head->first;
}

static inline int link_hlist_is_single(link_hlist_head_t *head) {
  return head->first && !head->first->next;
}

static inline void link_hlist_remove(link_hlist_node_t *node) {
  if (node->pprev) {
    link_hlist_node_t *next = node->next;
    link_hlist_node_t **pprev = node->pprev;
    *pprev = next;
    if(next) next->pprev = pprev;
  }
}

static inline void link_hlist_remove_init(link_hlist_node_t *node) {
  link_hlist_remove(node);
  link_hlist_node_init(node);
}

static inline void link_hlist_insert_head(link_hlist_node_t *node,
                                           link_hlist_head_t *head) {
  link_hlist_node_t *first = head->first;
  node->next = first;
  if (first) first->pprev = &node->next;
  head->first = node;
  node->pprev = &head->first;
}

#define link_hlist_entry(ptr, type, member) \
  container_of(ptr, type, member)

#define link_hlist_for_each(pos, head) \
  for (pos = (head)->first; pos ; pos = pos->next)

/**
  link_hlist_for_each_entry	- iterate over list of given type
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the link_hlist_node_t within the struct.
 */
#define link_hlist_for_each_entry(pos, head, member)				\
  for (pos = hlist_entry((head)->first, typeof(*(pos)), member);\
       pos;							\
       pos = hlist_entry((pos)->member.next, typeof(*(pos)), member))

#endif /*LINK_LIST_H*/
