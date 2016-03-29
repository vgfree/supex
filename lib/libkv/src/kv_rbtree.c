/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file   kv_rbtree.c
 * @brief  Red black tree general implementation.
 *
 * @author shishengjie
 * @date   2015-07-18
 */

#include "kv_inner.h"
#include "kv_rbtree.h"


static void default_free_cb(void *value)
{
        REDIS_NOTUSED(value);
//        zfree(value);
}

static void rbt_rotate_left(struct kv_rbtree *tree, struct kv_rbtree_node *cur)
{
        struct kv_rbtree_node *adjacent;

        if (!tree || !cur || cur == &tree->nil || cur->rchild == &tree->nil) {
                logicError("Invalid parameter passed to rbtree!");
        }

        adjacent = cur->rchild;
        adjacent->parent = cur->parent;
        if (cur->parent->lchild == cur)
                adjacent->parent->lchild = adjacent;
        else
                adjacent->parent->rchild = adjacent;
        cur->parent = adjacent;
        cur->rchild = adjacent->lchild;
        if (cur->rchild != &tree->nil) // ignore tree->nil node
                cur->rchild->parent = cur;
        adjacent->lchild = cur;
        
        if (tree->root == cur)
                tree->root = adjacent;
}

static void rbt_rotate_right(struct kv_rbtree *tree, struct kv_rbtree_node *cur)
{
        struct kv_rbtree_node *adjacent;

        if (!tree || !cur || cur == &tree->nil || cur->lchild == &tree->nil) {
                logicError("Invalid parameter passed to rbtree!");
        }

        adjacent = cur->lchild;
        adjacent->parent = cur->parent;
        if (cur->parent->lchild == cur)
                adjacent->parent->lchild = adjacent;
        else
                adjacent->parent->rchild = adjacent;
        cur->parent = adjacent;
        cur->lchild = adjacent->rchild;
        if (cur->lchild != &tree->nil) // ignore tree->nil node
                cur->lchild->parent = cur;
        adjacent->rchild = cur;
        
        if (tree->root == cur)
                tree->root = adjacent;
}

struct kv_rbtree* rbt_create(kv_rbt_free_cb free)
{
        struct kv_rbtree *tree;

        tree = zmalloc(sizeof(*tree));
        if (free == NULL)
                tree->free = default_free_cb;
        else
                tree->free = free;

        tree->count = 0;
        tree->root = &tree->nil;
        tree->nil.color = BLACK;
        tree->nil.lchild = tree->nil.rchild = &tree->nil;
        
        return tree;
}

void rbt_clear(struct kv_rbtree *tree)
{
        struct kv_rbtree_node *cur;

        cur = tree->root;
        while(cur != &tree->nil) {
                rbt_delete_from_key(tree, cur->key);
                cur = tree->root;
        }
        tree->count = 0;
}

void rbt_destroy(struct kv_rbtree *tree)
{
        rbt_clear(tree);
        zfree(tree);
}

struct kv_rbtree_node* rbt_search(struct kv_rbtree *tree, long long key)
{
        struct kv_rbtree_node *cur = tree->root;

        while(cur != &tree->nil) {
                if (cur->key == key) return cur;
                else if (key < cur->key)
                        cur = cur->lchild;
                else
                        cur = cur->rchild;
        }

        return NULL;
}

struct kv_rbtree_node* rbt_search_ge(struct kv_rbtree *tree, long long key)
{
        struct kv_rbtree_node *cur;

        cur = tree->root;
        while(cur != &tree->nil) {
                if (key >= cur->key) {
                        return cur;
                }
                cur = cur->rchild;
        }
        return NULL;
}

static inline struct kv_rbtree_node* uncle(struct kv_rbtree_node *node)
{
        return node->parent->parent->lchild == node->parent ?
                node->parent->parent->rchild : node->parent->parent->lchild;
}

#define IS_LL(node) (node->parent->parent->lchild == node->parent && node->parent->lchild == node)
#define IS_LR(node) (node->parent->parent->lchild == node->parent && node->parent->rchild == node)
#define IS_RR(node) (node->parent->parent->rchild == node->parent && node->parent->rchild == node)
#define IS_RL(node) (node->parent->parent->rchild == node->parent && node->parent->lchild == node)

static void _insert_fixup(struct kv_rbtree *tree, struct kv_rbtree_node *new_node)
{
        struct kv_rbtree_node *cur, *u;

        cur = new_node;
        while(cur->parent->color != BLACK) {
                u = uncle(cur);
                if (u->color == RED) { // uncle RED
                        cur->parent->color = BLACK;
                        u->color = BLACK;
                        cur->parent->parent->color = RED;
                        cur = cur->parent->parent;
                        continue;
                } else { // uncle BLACK
                        // LL|LR|RR|RL
                        if (IS_LL(cur)) {
                                cur->parent->color = BLACK;
                                cur->parent->parent->color = RED;
                                rbt_rotate_right(tree, cur->parent->parent);
                                break;
                        } else if (IS_LR(cur)) {
                                cur->parent->parent->color = RED;
                                cur->color = BLACK;
                                rbt_rotate_left(tree, cur->parent);
                                rbt_rotate_right(tree, cur->parent);
                                break;
                        } else if (IS_RR(cur)) {
                                cur->parent->color = BLACK;
                                cur->parent->parent->color = RED;
                                rbt_rotate_left(tree, cur->parent->parent);
                                break;
                        } else if (IS_RL(cur)) {
                                cur->color = BLACK;
                                cur->parent->parent->color = RED;
                                rbt_rotate_right(tree, cur->parent);
                                rbt_rotate_left(tree, cur->parent);
                                break;
                        } else {
                                logicError("Rbtree exception logical error!");
                        }
                }
        } // end while
        tree->root->color = BLACK;
}

//void rbt_insert(struct rbtree *tree, int key, const char *str)
void rbt_insert(struct kv_rbtree *tree, long long key, void *value)
{
        struct kv_rbtree_node *cur, *parent, *node;

        node = zmalloc(sizeof(struct kv_rbtree_node));

        node->key = key;
        node->value = value;
        node->color = RED;
        node->parent = node->lchild = node->rchild = &tree->nil;

        tree->count++; /** insert never failed */
        
        /** insert new node to tree */
        parent = &tree->nil;
        cur = tree->root;
        while(cur != &tree->nil) {
                parent = cur;
                if (key < cur->key)
                        cur = cur->lchild;
                else
                        cur = cur->rchild;
        }
        
        node->parent = parent;
        if (parent == &tree->nil) { // root
                tree->root = node;
                node->color = BLACK;
                return;
        } else if (key < parent->key) {
                parent->lchild = node;
        } else {
                parent->rchild = node;
        }

        _insert_fixup(tree, node);
}

/**
 * Adjust to balance after the node is deleted.
 */
static void _delete_fixup(struct kv_rbtree *tree, struct kv_rbtree_node *x_node)
{
        int temp;
        struct kv_rbtree_node *w_node, *p_node;

        while(1) {
                p_node = x_node->parent;
                w_node = p_node->lchild == x_node ? p_node->rchild : p_node->lchild;
                if (p_node == &tree->nil) { /** root */
                        x_node->color = BLACK;
                        return;
                }
                
                //case1(w is red)
                if (w_node->color == RED) {
                        temp = p_node->color;
                        p_node->color = w_node->color;
                        w_node->color = temp;

                        if (p_node->lchild == x_node)
                                rbt_rotate_left(tree, p_node);
                        else
                                rbt_rotate_right(tree, p_node);
                        continue;
                }
                //case2
                else if (w_node->color == BLACK
                         && w_node->lchild->color == BLACK
                         && w_node->rchild->color == BLACK) {
                        w_node->color = RED;
                        if (p_node->color == RED) {
                                p_node->color = BLACK;
                                return; /** done */
                        } else {
                                /** continue double black node */
                                x_node = p_node;
                                if (x_node->parent == &tree->nil)
                                        return; /** root */
                                
                                continue;
                        }
                }
                //case3 (turn to case4)
                else if (x_node->parent->lchild == x_node
                         && w_node->color == BLACK
                         && w_node->lchild->color == RED
                         && w_node->rchild->color == BLACK) { // x->left
                        w_node->color = RED;
                        w_node->lchild->color = BLACK;
                        rbt_rotate_right(tree, w_node);

                        continue;
                }
                else if (x_node->parent->rchild == x_node
                         && w_node->color == BLACK
                         && w_node->lchild->color == BLACK
                         && w_node->rchild->color == RED) { // x->right

                        temp = w_node->color;
                        w_node->color = w_node->rchild->color;
                        w_node->rchild->color = temp;

                        rbt_rotate_left(tree, w_node);
                        continue;
                }
                //case4
                else if (x_node->parent->lchild == x_node
                         && w_node->color == BLACK
                         && w_node->rchild->color == RED) { // left
                        w_node->rchild->color = BLACK;

                        temp = p_node->color;
                        p_node->color = w_node->color;
                        w_node->color = temp;

                        rbt_rotate_left(tree, p_node);
                        return; /** done */
                }
                else if (x_node->parent->rchild == x_node
                         && w_node->color == BLACK
                         && w_node->lchild->color == RED) {
                        w_node->lchild->color = BLACK;

                        temp = p_node->color;
                        p_node->color = w_node->color;
                        w_node->color = temp;

                        rbt_rotate_right(tree, p_node);
                        return; /** done */
                }
                //abort
                else {
                        logicError("Rbtree exception logical error!");
                }
        }
}

static void _rbt_delete(struct kv_rbtree *tree, struct kv_rbtree_node *node)
{
        char temp_c; 
        struct kv_rbtree_node *z_node, *y_node, *x_node, *temp;

        z_node = node;
        if (!z_node) {
                printf("===>Delete failed, not found!\n");
                return;
        }

        tree->count--; /** delete never failed */
        
        /** single branch */
        if ((z_node->lchild != &tree->nil && z_node->rchild == &tree->nil) ||
            (z_node->lchild == &tree->nil && z_node->rchild != &tree->nil))
        {
                if (z_node->lchild != &tree->nil)
                        x_node = z_node->lchild;
                else
                        x_node = z_node->rchild;
                x_node->parent = z_node->parent;
                if (z_node->parent->lchild == z_node) {
                        z_node->parent->lchild = x_node;
                } else {
                        z_node->parent->rchild = x_node;
                }
                x_node->color = BLACK;
                if (x_node->parent == &tree->nil)
                        tree->root = x_node;

                tree->free(z_node->value);
                zfree(z_node);
                return;
        } else if (z_node->lchild == &tree->nil && z_node->rchild == &tree->nil) {
                x_node = z_node;
                if (x_node->parent == &tree->nil) {/** root */
                        tree->root = &tree->nil;
                        tree->free(x_node->value);
                        zfree(x_node);
                        return;
                } else {
                        if (x_node->parent->lchild == x_node) {
                                x_node->parent->lchild = &tree->nil;
                                tree->nil.parent = x_node->parent;
                                
                        } else {
                                x_node->parent->rchild = &tree->nil;
                                tree->nil.parent = x_node->parent;
                        }

                        if (x_node->color == RED) {
                                tree->free(x_node->value);
                                zfree(x_node);
                                return;
                        } else {
                                tree->free(x_node->value);
                                zfree(x_node);
                                x_node = &tree->nil;
                                goto del_fixup;
                        }
                }
        } else {
                /** search successor node */
                y_node = &tree->nil;
                temp = z_node->rchild;
                while(temp != &tree->nil) {
                        y_node = temp;
                        temp = temp->lchild;
                }      
                
                z_node->key = y_node->key;
                z_node->value = y_node->value; /** fix remove bug */
                temp_c = y_node->color;
                
                if (y_node->parent->lchild == y_node)
                        y_node->parent->lchild = y_node->rchild;
                else
                        y_node->parent->rchild = y_node->rchild;
                
                y_node->rchild->parent = y_node->parent;

                x_node = y_node->rchild;
                tree->free(y_node->value);
                zfree(y_node);

                if (temp_c == RED) return; /** leaf node */
                if (x_node != &tree->nil) {
                        x_node->color = BLACK;
                        return;
                }

                goto del_fixup;
        }

del_fixup:
        _delete_fixup(tree, x_node);
}

void rbt_delete_from_key(struct kv_rbtree *tree, long long key)
{
        _rbt_delete(tree, rbt_search(tree, key));
}

void rbt_delete_from_node(struct kv_rbtree *tree, struct kv_rbtree_node *node)
{
        _rbt_delete(tree, node);
}
