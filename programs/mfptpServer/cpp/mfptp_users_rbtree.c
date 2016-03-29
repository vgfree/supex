/*
 * 版权:     暂无
 * 文件名称: mfptp_users_rbtree.c
 * 创建者  : 罗开辉
 * 创建日期: 2015/03/06
 * 文件描述: 使用RBTREE管理mfptp协议中连接的用户
 * 历史记录: 1. 2015/3/7,新建立by l00167671/luokaihui
 */

#include "mfptp_users_rbtree.h"

/* 名      称: rb_insert_user
 * 功      能: 插入用户
 * 参      数: root, 红黑树指针
 *             user, 要插入红黑树的用户指针
 * 返回值:  TRUE表示成功，FALSE表示失败
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int rb_insert_user(rb_root *root, struct user_info *user)
{
	rb_node                 **cur = &(root->rb_node);
	rb_node                 *parent = NULL;
	struct user_info        *me = NULL;
	int                     result;

	/* 查找插入位置 */
	while (*cur) {
		me = rb_entry(*cur, struct user_info, node);
		result = strcmp(user->who, me->who);
		parent = *cur;

		if (result < 0) {
			cur = &((*cur)->rb_left);
		} else if (result > 0) {
			cur = &((*cur)->rb_right);
		} else {
			return FALSE;
		}
	}

	/* 插入节点，重新平衡*/
	rb_link_node(&user->node, parent, cur);
	rb_insert_color(&user->node, root);

	return TRUE;
}

/* 名      称: rb_remove_user
 * 功      能: 删除红黑树中的用户
 * 参      数: root, 红黑树指针
 *             who,  用户标识符
 * 返回值: TRUE, 删除成功; FALSE, 删除失败
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int rb_remove_user(rb_root *root, char *who)
{
	struct user_info *user = rb_search_user(root, who);

	if (user) {
		rb_erase(&user->node, root);
		/* 释放user结构体*/
		free(user);
		return TRUE;
	} else {
		return FALSE;
	}
}

/* 名      称: rb_search_user
 * 功      能: 根据用户标识符，查找用户
 * 参      数:
 * 返回值:  成功返回user_info 结构体指针，失败返回NULL
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
struct user_info *rb_search_user(rb_root *root, char *who)
{
	rb_node *node = root->rb_node;

	while (node) {
		struct user_info        *user = rb_entry(node, struct user_info, node);
		int                     result;

		result = strcmp(who, user->who);

		if (result < 0) {
			node = node->rb_left;
		} else if (result > 0) {
			node = node->rb_right;
		} else {
			return user;
		}
	}

	return NULL;
}

/* 名      称: __print_DLR
 * 功      能: 前序遍历红黑树
 * 参      数: node, 开始遍历的节点
 * 返 回 值  : 无
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
void __print_DLR(rb_node *node)
{
	struct user_info *p = NULL;

	if (node == NULL) {
		return;
	}

	p = rb_entry(node, struct user_info, node);
	__print_DLR(node->rb_left);
	__print_DLR(node->rb_right);
}

/* 名      称: __print_LDR
 * 功      能: 中序遍历红黑树
 * 参      数: node, 开始遍历的节点
 * 返 回 值:   无
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
void __print_LDR(rb_node *node)
{
	struct user_info *p = NULL;

	if (node == NULL) {
		return;
	}

	__print_LDR(node->rb_left);
	p = rb_entry(node, struct user_info, node);
	__print_LDR(node->rb_right);
}

/* 名      称: __print_LRD
 * 功      能: 后序遍历红黑树
 * 参      数: node, 开始遍历的节点
 * 返 回 值:   无
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
void __print_LRD(rb_node *node)
{
	struct user_info *p = NULL;

	if (node == NULL) {
		return;
	}

	__print_LRD(node->rb_left);
	__print_LRD(node->rb_right);
	p = rb_entry(node, struct user_info, node);
}

/* 名      称: __print_LRD
 * 功      能: 后序遍历红黑树
 * 参      数: trav, 遍历方式。取值有三种,
 *             EN_RB_PREORDER, 前序遍历
 *             EN_RB_INORDER,  中序遍历
 *             EN_RB_POSTORDER,后序遍历
 *             root, 红黑树根节点
 * 返 回 值:  无
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
void rb_print_tree(rb_traversal trav, rb_root root)
{
	if (root.rb_node == NULL) {
		return;
	}

	if (EN_RB_PREORDER == trav) {
		__print_DLR(root.rb_node);
	} else if (EN_RB_INORDER == trav) {
		__print_LDR(root.rb_node);
	} else if (EN_RB_POSTORDER == trav) {
		__print_LRD(root.rb_node);
	}
}

