/* 文件: rbtree.h
 *     版权:
 *     描述: 本文件提供红黑树节点和根节点的定义以及
 *                   接口声明
 *     历史: 2015/3/7 新文件建立by l00167671/luokaihui
 */
#ifndef __MFPTP_RBTREE_H__
#define __MFPTP_RBTREE_H__

#include <stdio.h>

/*******************************常量定义*************************************/
#define RB_RED          0
#define RB_BLACK        1

/*******************************结构体****************************************/
typedef struct rb_node_s
{
	struct rb_node_s        *rb_parent;
	int                     rb_color;
	struct rb_node_s        *rb_right;
	struct rb_node_s        *rb_left;
} rb_node;

typedef struct rb_root_s
{
	struct rb_node_s *rb_node;
} rb_root;

#define RB_ROOT (rb_root) {NULL, }
#define rb_entry(ptr, type, member) \
	((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/*********************************函数声明**********************************/
extern void rb_insert_color(rb_node *rnode, rb_root *r);

extern void rb_erase(rb_node *rnode, rb_root *r);

static inline void rb_link_node(rb_node *node, rb_node *parent, rb_node **rb_link)
{
	node->rb_parent = parent;
	node->rb_color = RB_RED;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}
#endif	/* ifndef __MFPTP_RBTREE_H__ */

