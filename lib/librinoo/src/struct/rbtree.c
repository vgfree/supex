/**
 * @file   rbtree.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Fri Apr 13 10:55:43 2012
 *
 * @brief Red-Black tree implementation.
 *
 *
 */
/*-
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "rinoo/struct/module.h"

static inline void rbtree_rotate_left(t_rbtree *tree, t_rbtree_node *node)
{
	t_rbtree_node *current;

	current = node->right;
	node->right = current->left;
	if (node->right != NULL) {
		current->left->parent = node;
	}
	current->parent = node->parent;
	if (current->parent != NULL) {
		if (node == node->parent->left) {
			node->parent->left = current;
		} else {
			node->parent->right = current;
		}
	} else {
		tree->root = current;
	}
	current->left = node;
	node->parent = current;
}

static inline void rbtree_rotate_right(t_rbtree *tree, t_rbtree_node *node)
{
	t_rbtree_node *current;

	current = node->left;
	node->left = current->right;
	if (node->left != NULL) {
		current->right->parent = node;
	}
	current->parent = node->parent;
	if (current->parent != NULL) {
		if (node == node->parent->left) {
			node->parent->left = current;
		} else {
			node->parent->right = current;
		}
	} else {
		tree->root = current;
	}
	current->right = node;
	node->parent = current;
}

static void rbtree_insert_color(t_rbtree *tree, t_rbtree_node *node)
{
	t_rbtree_node *parent;
	t_rbtree_node *gparent;
	t_rbtree_node *current;

	parent = node->parent;
	while ((parent = node->parent) != NULL &&
	       parent->color == RINOO_RBTREE_RED &&
	       (gparent = parent->parent) != NULL) {

		if (parent == gparent->left) {
			current = gparent->right;
			if (current != NULL && current->color == RINOO_RBTREE_RED) {
				current->color = RINOO_RBTREE_BLACK;
				parent->color = RINOO_RBTREE_BLACK;
				gparent->color = RINOO_RBTREE_RED;
				node = gparent;
			} else {
				if (parent->right == node) {
					rbtree_rotate_left(tree, parent);
					current = parent;
					parent = node;
					node = current;
				}
				parent->color = RINOO_RBTREE_BLACK;
				gparent->color = RINOO_RBTREE_RED;
				rbtree_rotate_right(tree, gparent);
			}
		} else {
			current = gparent->left;
			if (current != NULL && current->color == RINOO_RBTREE_RED) {
				current->color = RINOO_RBTREE_BLACK;
				parent->color = RINOO_RBTREE_BLACK;
				gparent->color = RINOO_RBTREE_RED;
				node = gparent;
			} else {
				if (parent->left == node) {
					rbtree_rotate_right(tree, parent);
					current = parent;
					parent = node;
					node = current;
				}
				parent->color = RINOO_RBTREE_BLACK;
				gparent->color = RINOO_RBTREE_RED;
				rbtree_rotate_left(tree, gparent);
			}
		}
	}
	tree->root->color = RINOO_RBTREE_BLACK;
}

static void rbtree_remove_color(t_rbtree *tree, t_rbtree_node *parent, t_rbtree_node *node)
{
	t_rbtree_node *current;

	while ((node == NULL || node->color == RINOO_RBTREE_BLACK) && node != tree->root && parent != NULL) {
		if (parent->left == node) {
			current = parent->right;
			if (current->color == RINOO_RBTREE_RED) {
				current->color = RINOO_RBTREE_BLACK;
				parent->color = RINOO_RBTREE_RED;
				rbtree_rotate_left(tree, parent);
				current = parent->right;
			}
			if ((current->left == NULL || current->left->color == RINOO_RBTREE_BLACK) &&
			    (current->right == NULL || current->right->color == RINOO_RBTREE_BLACK)) {
				current->color = RINOO_RBTREE_RED;
				node = parent;
				parent = node->parent;
			} else {
				if (current->right == NULL || current->right->color == RINOO_RBTREE_BLACK) {
					if (current->left != NULL) {
						current->left->color = RINOO_RBTREE_BLACK;
					}
					current->color = RINOO_RBTREE_RED;
					rbtree_rotate_right(tree, current);
					current = parent->right;
				}
				current->color = parent->color;
				parent->color = RINOO_RBTREE_BLACK;
				if (current->right != NULL) {
					current->right->color = RINOO_RBTREE_BLACK;
				}
				rbtree_rotate_left(tree, parent);
				node = tree->root;
				break;
			}
		} else {
			current = parent->left;
			if (current->color == RINOO_RBTREE_RED) {
				current->color = RINOO_RBTREE_BLACK;
				parent->color = RINOO_RBTREE_RED;
				rbtree_rotate_right(tree, parent);
				current = parent->left;
			}
			if ((current->left == NULL || current->left->color == RINOO_RBTREE_BLACK) &&
			    (current->right == NULL || current->right->color == RINOO_RBTREE_BLACK)) {
				current->color = RINOO_RBTREE_RED;
				node = parent;
				parent = node->parent;
			} else {
				if (current->left == NULL || current->left->color == RINOO_RBTREE_BLACK) {
					if (current->right != NULL) {
						current->right->color = RINOO_RBTREE_BLACK;
					}
					current->color = RINOO_RBTREE_RED;
					rbtree_rotate_left(tree, current);
					current = parent->left;
				}
				current->color = parent->color;
				parent->color = RINOO_RBTREE_BLACK;
				if (current->left != NULL) {
					current->left->color = RINOO_RBTREE_BLACK;
				}
				rbtree_rotate_right(tree, parent);
				node = tree->root;
				break;
			}
		}
	}

	if (node != NULL) {
		node->color = RINOO_RBTREE_BLACK;
	}
}

int rbtree(t_rbtree *tree, int (*compare)(t_rbtree_node *node1, t_rbtree_node *node2), void (*delete)(t_rbtree_node *node))
{
	XASSERT(tree != NULL, -1);
	XASSERT(compare != NULL, -1);

	tree->root = NULL;
	tree->head = NULL;
	tree->compare = compare;
	tree->delete = delete;
	return 0;
}

void rbtree_flush(t_rbtree *tree)
{
	t_rbtree_node *old;
	t_rbtree_node *current;

	XASSERTN(tree != NULL);

	if (tree->root != NULL && tree->delete != NULL) {
		current = tree->head;
		while (current != NULL) {
			if (current->right != NULL) {
				current = current->right;
				while (current->left != NULL) {
					current = current->left;
				}
			} else {
				if (current->parent != NULL && (current == current->parent->left)) {
					old = current;
					current = current->parent;
					tree->delete(old);
				} else {
					while (current->parent != NULL && (current == current->parent->right)) {
						old = current;
						current = current->parent;
						tree->delete(old);
					}
					old = current;
					current = current->parent;
					tree->delete(old);
				}
			}
		}
	}
	tree->size = 0;
	tree->root = NULL;
	tree->head = NULL;
}

int rbtree_put(t_rbtree *tree, t_rbtree_node *node)
{
	int cmp;
	int head;
	t_rbtree_node *parent;
	t_rbtree_node *current;

	XASSERT(tree != NULL, -1);
	XASSERT(node != NULL, -1);

	cmp = 0;
	head = 1;
	parent = NULL;
	current = tree->root;
	while (current != NULL) {
		parent = current;
		cmp = tree->compare(node, parent);
		if (cmp < 0) {
			current = current->left;
		} else if (likely(cmp > 0)) {
			current = current->right;
			head = 0;
		} else {
			return -1;
		}
	}
	node->left = NULL;
	node->right = NULL;
	node->parent = parent;
	node->color = RINOO_RBTREE_RED;
	if (parent != NULL) {
		if (cmp < 0) {
			parent->left = node;
		} else {
			parent->right = node;
			head = 0;
		}
	} else {
		tree->root = node;
	}
	if (head != 0) {
		tree->head = node;
	}

	rbtree_insert_color(tree, node);
	tree->size++;

	return 0;
}

void rbtree_remove(t_rbtree *tree, t_rbtree_node *node)
{
	t_rbtree_node *old;
	t_rbtree_node *child;
	t_rbtree_node *parent;
	t_rbtree_color color;

	XASSERTN(tree != NULL);
	XASSERTN(node != NULL);

	old = node;
	if (node->left == NULL) {
		child = node->right;
	} else if (node->right == NULL) {
		child = node->left;
	} else {
		node = node->right;
		while (node->left != NULL) {
			node = node->left;
		}
		child = node->right;
		parent = node->parent;
		color = node->color;
		if (child != NULL) {
			child->parent = parent;
		}
		if (parent != NULL) {
			if (parent->left == node) {
				parent->left = child;
			} else {
				parent->right = child;
			}
		} else {
			tree->root = child;
		}
		if (node->parent == old) {
			parent = node;
		}
		*node = *old;
		if (old->parent != NULL) {
			if (old->parent->left == old) {
				old->parent->left = node;
			} else {
				old->parent->right = node;
			}
		} else {
			tree->root = node;
		}
		old->left->parent = node;
		if (old->right != NULL) {
			old->right->parent = node;
		}
		goto color;
	}
	parent = node->parent;
	color = node->color;
	if (child != NULL) {
		child->parent = parent;
	}
	if (parent != NULL) {
		if (parent->left == node) {
			parent->left = child;
		} else {
			parent->right = child;
		}
	} else {
		tree->root = child;
	}

	if (node == tree->head) {
		if (parent != NULL && parent->left == NULL) {
			tree->head = parent;
		} else {
			tree->head = child;
		}
	}
color:
	if (color == RINOO_RBTREE_BLACK) {
		rbtree_remove_color(tree, parent, child);
	}

	if (tree->delete != NULL) {
		tree->delete(old);
	}
	tree->size--;
}

t_rbtree_node *rbtree_head(t_rbtree *tree)
{
	XASSERT(tree != NULL, NULL);

	return tree->head;
}

t_rbtree_node *rbtree_next(t_rbtree_node *node)
{
	if (node->right != NULL) {
		node = node->right;
		while (node->left != NULL) {
			node = node->left;
		}
	} else {
		if (node->parent != NULL && (node == node->parent->left)) {
			node = node->parent;
		} else {
			while (node->parent != NULL && (node == node->parent->right)) {
				node = node->parent;
			}
			node = node->parent;
		}
	}

	return node;
}

t_rbtree_node *rbtree_find(t_rbtree *tree, t_rbtree_node *node)
{
	int cmp;
	t_rbtree_node *tmp;

	XASSERT(tree != NULL, NULL);
	XASSERT(node != NULL, NULL);

	for (tmp = tree->root; tmp != NULL;) {
		cmp = tree->compare(node, tmp);
		if (cmp < 0) {
			tmp = tmp->left;
		} else if (cmp > 0) {
			tmp = tmp->right;
		} else {
			return tmp;
		}
	}

	return NULL;
}
