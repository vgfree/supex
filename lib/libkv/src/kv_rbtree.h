/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file   kv_rbtree.h
 * @brief  Red black tree declarations.
 *
 * @author shishengjie
 * @date   2015-07-18
 */

#ifndef KV_RBTREE_H_
#define KV_RBTREE_H_




#define RED   0
#define BLACK 1



typedef void (*kv_rbt_free_cb)(void *value);


struct kv_rbtree_node {
        long long key;
        const char *str;
        void *value;
        char color;
        struct kv_rbtree_node *parent;
        struct kv_rbtree_node *lchild, *rchild;
};

struct kv_rbtree {
        kv_rbt_free_cb free;
        int count;
        struct kv_rbtree_node *root;
        struct kv_rbtree_node nil;
};



struct kv_rbtree* rbt_create(kv_rbt_free_cb free);
void rbt_clear(struct kv_rbtree *tree);
void rbt_destroy(struct kv_rbtree *tree);
void rbt_delete_from_key(struct kv_rbtree *tree, long long key);
void rbt_delete_from_node(struct kv_rbtree *tree, struct kv_rbtree_node *node);
void rbt_insert(struct kv_rbtree *tree, long long key, void *value);
struct kv_rbtree_node* rbt_search(struct kv_rbtree *tree, long long key);
struct kv_rbtree_node* rbt_search_ge(struct kv_rbtree *tree, long long key);





#endif
