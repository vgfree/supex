/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file   kv_expire.c
 * @brief  Expire keys management implementation.
 *
 * @author shishengjie
 * @date   2015-07-18
 */

#include "kv_inner.h"
#include "kv_rbtree.h"


kv_expire_manager_t* kv_expire_manager_create()
{
        kv_expire_manager_t *manager;

        manager = zmalloc(sizeof(*manager));
        manager->ex_tree = rbt_create(NULL);

        return manager;
}

void kv_expire_manager_destroy(struct kv_expire_manager *manager)
{
        rbt_destroy(manager->ex_tree);
        zfree(manager);
}

void kv_expire_manager_add(struct kv_handler *handler, long long key, void *value)
{
        kv_expire_manager_t *manager = handler->db[handler->dbindex].expire_manager;
        rbt_insert(manager->ex_tree, key, value);
}

int kv_expire_manager_remove(struct kv_handler *handler, long long count)
{
        time_t now;
        long long del = 0; 
        struct kv_rbtree_node* cur;
        struct kv_expire_manager *manager = handler->db[handler->dbindex].expire_manager;
        now = mstime();
        if (count < 0) count = LLONG_MAX;
        while(count > 0 && (cur = rbt_search_ge(manager->ex_tree, now)) != NULL) {
                dictDelete(handler->db[handler->dbindex].expires, cur->value);
                dictDelete(handler->db[handler->dbindex].dict, cur->value);
                rbt_delete_from_key(manager->ex_tree, cur->key);
                count--;
                del++;
        }
        return del;
}

int kv_expire_manager_remove_complete(struct kv_handler *handler, long long key, void *value)
{
        struct kv_rbtree_node *node;
        kv_expire_manager_t *manager = handler->db[handler->dbindex].expire_manager;

        node = rbt_search(manager->ex_tree, key);
        if (node == NULL) {
                return 0;
        } else {
                while(node->key == key && node != &manager->ex_tree->nil) {
                        /** compare with value */
                        if (dictSdsKeyCompare(NULL, value, node->value)) {
                                rbt_delete_from_node(manager->ex_tree, node);
                                return 1;
                        }
                        node = node->rchild;
                }
                return 0;
        }
}

void kv_expire_manager_clear(struct kv_handler *handler, struct kv_expire_manager *manager)
{
        REDIS_NOTUSED(handler);
//        struct kv_expire_manager *manager = handler->db[handler->dbindex].expire_manager;
        
        rbt_clear(manager->ex_tree);
}
