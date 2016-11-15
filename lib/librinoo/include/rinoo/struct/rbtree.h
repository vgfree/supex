/**
 * @file   rbtree.h
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Fri Apr 13 10:43:43 2012
 *
 * @brief Header file for Red-Black tree implementation.
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

#ifndef RINOO_STRUCT_RBTREE_H_
#define RINOO_STRUCT_RBTREE_H_

typedef enum e_rbtree_color {
	RINOO_RBTREE_RED = 0,
	RINOO_RBTREE_BLACK
} t_rbtree_color;

typedef struct s_rbtree_node {
	t_rbtree_color color;
	struct s_rbtree_node *left;
	struct s_rbtree_node *right;
	struct s_rbtree_node *parent;
} t_rbtree_node;

typedef struct s_rbtree {
	uint64_t size;
	t_rbtree_node *root;
	t_rbtree_node *head;
	int (*compare)(t_rbtree_node *node1, t_rbtree_node *node2);
	void (*delete)(t_rbtree_node *node);
} t_rbtree;

int rbtree(t_rbtree *tree,
		int (*compare)(t_rbtree_node *node1, t_rbtree_node *node2),
		void (*delete)(t_rbtree_node *node));
void rbtree_flush(t_rbtree *tree);
int rbtree_put(t_rbtree *tree, t_rbtree_node *node);
void rbtree_remove(t_rbtree *tree, t_rbtree_node *node);
t_rbtree_node *rbtree_head(t_rbtree *tree);
t_rbtree_node *rbtree_next(t_rbtree_node *node);
t_rbtree_node *rbtree_find(t_rbtree *tree, t_rbtree_node *node);

#endif /* !RINOO_STRUCT_RBTREE_H_ */
