/**
 * @file   list_put.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Fri Apr 13 14:58:37 2014
 *
 * @brief  rinoo list add unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#define RINOO_LISTTEST_NB_ELEM 10000

typedef struct mytest
{
	int val;
	t_list_node node;
} tmytest;

int cmp_func(t_list_node *node1, t_list_node *node2)
{
	tmytest *a = container_of(node1, tmytest, node);
	tmytest *b = container_of(node2, tmytest, node);

	return (a->val == b->val ? 0 : (a->val > b->val ? 1 : -1));
}

int main()
{
	int i;
	int min;
	int last;
	tmytest *head;
	t_list mylist;
	t_list_node *node;
	tmytest tab[RINOO_LISTTEST_NB_ELEM];

	XTEST(list(&mylist, cmp_func) == 0);
	for (i = 0; i < RINOO_LISTTEST_NB_ELEM; i++) {
		tab[i].val = random();
		if (i == 0 || min > tab[i].val) {
			min = tab[i].val;
		}
		list_put(&mylist, &tab[i].node);
		XTEST(mylist.head != NULL);
		head = container_of(mylist.head, tmytest, node);
		XTEST(head->val == min);
	}
	XTEST(list_size(&mylist) == RINOO_LISTTEST_NB_ELEM);
	last = head->val;
	for (i = 0; i < RINOO_LISTTEST_NB_ELEM; i++) {
		node = list_pop(&mylist);
		XTEST(node != NULL);
		head = container_of(node, tmytest, node);
		XTEST(head->val >= last);
		last = head->val;
	}
	XPASS();
}
