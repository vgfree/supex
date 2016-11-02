#include "user_rbtree.h"

/* 名      称: rb_insert_user_record
 * 功      能: 插入用户红黑树中
 * 参      数:
 * 返回值:  TRUE表示成功，FALSE表示失败
 */
int rb_insert_user_record(rb_root *root, struct user_record *user)
{
	rb_node                 **cur = &(root->rb_node);
	rb_node                 *parent = NULL;
	struct user_record      *me = NULL;
	int                     result;

	/* 查找插入位置 */
	while (*cur) {
		me = rb_entry(*cur, struct user_record, user_node);
		// result = strcmp(user->who, me->who);
		result = user->user_id - me->user_id;

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
	rb_link_node(&user->user_node, parent, cur);
	rb_insert_color(&user->user_node, root);

	return TRUE;
}

/* 名      称: rb_remove_user_record
 * 功      能:
 * 参      数:
 * 返回值:
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int rb_remove_user_record(rb_root *root, uint64_t imei)
{
	struct user_record *user = rb_search_user_record(root, imei);

	if (user) {
		rb_erase(&user->user_node, root);
		/* 释放user结构体*/
		return TRUE;
	} else {
		return FALSE;
	}
}

/* 名      称: rb_search_user_record
 * 功      能: 根据用户标识符，查找用户
 * 参      数:
 * 返回值:  成功返回user_info 结构体指针，失败返回NULL
 * 修      改:
 */
struct user_record *rb_search_user_record(rb_root *root, uint64_t imei)
{
	rb_node *node = root->rb_node;

	while (node) {
		struct user_record      *user = rb_entry(node, struct user_record, user_node);
		int                     result;

		// result = strcmp(who, user->who);
		result = imei - user->user_id;

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

rb_root g_user_info_tree = { NULL };

