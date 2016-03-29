#ifndef __MFPTP_USERS_RBTREE_H__
#define __MFPTP_USERS_RBTREE_H__

/*
 *   版权:
 *   描述: 使用RBTREE管理mfptp协议中连接的用户
 *   历史: 1. 2015/3/7,新建立by l00167671/luokaihui
 */
#include <stdlib.h>
#include <malloc.h>
#include "rbtree.h"
#include "mfptp_def.h"
typedef enum
{
	EN_RB_PREORDER = 0,
	EN_RB_INORDER = 1,
	EN_RB_POSTORDER = 2
} rb_traversal;

/* 名      称: rb_insert_user
 * 功      能: 插入用户
 * 参      数:
 * 返回值:  TRUE表示成功，FALSE表示失败
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int rb_insert_user(rb_root *root, struct user_info *user);

/* 名      称: rb_remove_user
 * 功      能:
 * 参      数:
 * 返回值:
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
int rb_remove_user(rb_root *root, char *who);

/* 名      称: rb_search_user
 * 功      能: 根据用户标识符，查找用户
 * 参      数:
 * 返回值:  成功返回user_info 结构体指针，失败返回NULL
 * 修      改: 新生成函数l00167671 at 2015/2/28
 */
struct user_info        *rb_search_user(rb_root *root, char *who);
#endif	/* ifndef __MFPTP_USERS_RBTREE_H__ */

