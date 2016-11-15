/**
 * @file   rbtree_head.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Fri Apr 13 14:58:37 2012
 *
 * @brief  rinoo rbtree head unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#define RINOO_HEADTEST_NB_ELEM		10000

typedef struct mytest
{
	int val;
	t_rbtree_node node;
} tmytest;

int cmp_func(t_rbtree_node *node1, t_rbtree_node *node2)
{
	tmytest *a = container_of(node1, tmytest, node);
	tmytest *b = container_of(node2, tmytest, node);

	return (a->val == b->val ? 0 : (a->val > b->val ? 1 : -1));
}

int main()
{
	int i;
	int min;
	tmytest *head;
	t_rbtree tree;
	t_rbtree_node *node;
	tmytest tab[RINOO_HEADTEST_NB_ELEM];

	XTEST(rbtree(&tree, cmp_func, NULL) == 0);
	for (i = 0; i < RINOO_HEADTEST_NB_ELEM; i++) {
		tab[i].val = random();
		if (i == 0 || min > tab[i].val) {
			min = tab[i].val;
		}
		XTEST(rbtree_put(&tree, &tab[i].node) == 0);
		XTEST(tree.head != NULL);
		head = container_of(tree.head, tmytest, node);
		XTEST(head->val == min);
	}
	rbtree_flush(&tree);
	for (i = 0; i < RINOO_HEADTEST_NB_ELEM; i++) {
		tab[i].val = i;
		XTEST(rbtree_put(&tree, &tab[i].node) == 0);
		XTEST(tree.head != NULL);
	}
	for (i = 0; i < RINOO_HEADTEST_NB_ELEM; i++) {
		node = rbtree_head(&tree);
		XTEST(node != NULL);
		head = container_of(node, tmytest, node);
		XTEST(head->val == i);
		rbtree_remove(&tree, node);
	}
	XPASS();
}
