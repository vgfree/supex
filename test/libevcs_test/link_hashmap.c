/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "link_hashmap.h"

/*double slots size and rehash*/
static void link_hashmap_rehash(link_hashmap_t *hash) {
  size_t old_size = 1 << hash->bits;
  size_t bits = hash->bits++;
  size_t size = 1 << bits;
  hash->max_items = size;
  link_hlist_head_t *old_slots = hash->slots;
  link_hlist_head_t *new_slots = link_malloc(sizeof(link_hlist_head_t) * size);
  for (size_t i = 0; i < size; i++) {
    new_slots[i].first = NULL;
  }

  link_hlist_head_t *current_slot;
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;
  size_t index;
  for (size_t i = 0; i < old_size; i++) {
    current_slot = &old_slots[i];
    while (!link_hlist_is_empty(current_slot)) {
      pos = current_slot->first;
      link_hlist_remove(pos);
      item = link_hlist_entry(pos, link_hashmap_item_t, node);
      index = link_hashmap_slot(item->hash, bits);
      link_hlist_insert_head(&item->node, &new_slots[index]);
    }
  }

  hash->slots = new_slots;
  link_free(old_slots);
}

link_hashmap_t *link_hashmap__create(size_t bits, 
                                     link_hashmap_strcmp_fn strcmp_fn, 
                                     link_hashmap_fn hash_fn) {
  link_hashmap_t *hash = link_malloc(sizeof(link_hashmap_t));
  if (hash == NULL) return NULL;

  hash->bits = bits;
  size_t size = 1 << bits;
  hash->max_items = size;
  hash->str_cmp = strcmp_fn;
  hash->hash = hash_fn;
  hash->items = 0;
  link_hlist_head_t *slots = link_malloc(sizeof(link_hlist_head_t) * size);
  if (slots == NULL) {
    link_free(hash);
    return NULL;
  }

  for (size_t i = 0; i < size; i++) {
    slots[i].first = NULL;
  }
  hash->slots = slots;
  return hash;
}

void link_hashmap_destroy(link_hashmap_t *hash) {
  if (hash == NULL) return;

  size_t size = 1 << hash->bits;
  link_hlist_head_t *slots = hash->slots;

  link_hlist_head_t *current_slot;
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;
  for (size_t i = 0; i < size; i++) {
    current_slot = &slots[i];
    while (!link_hlist_is_empty(current_slot)) {
      pos = current_slot->first;
      link_hlist_remove(pos);
      item = link_hlist_entry(pos, link_hashmap_item_t, node);
      if(item->pointer) link_pfree(item->pointer); 
      if(item->key) link_pfree(item->key);
      link_pfree(item);
    }
  }

  link_free(slots);
  link_free(hash);
}

void link_hashmap_str_set(link_hashmap_t *hash, char *key, size_t n, void *pointer) {
  if (hash->items >= hash->max_items) {
    link_hashmap_rehash(hash);
  }

  size_t hash_key = hash->hash(key, n);
  size_t index = link_hashmap_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;
  int cmp;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_hlist_entry(pos, link_hashmap_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          link_pfree(item->pointer);
          item->pointer = pointer;
          return;
        }
      }
    }
  }

  item = link_palloc(sizeof(link_hashmap_item_t));
  item->pointer = pointer;
  item->hash = hash_key;
  item->key_length = n;
  item->key = key;
  link_hlist_insert_head(&item->node, slot);
  hash->items++;
}

void *link_hashmap_str_get(link_hashmap_t *hash, const char *key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = link_hashmap_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;
  int cmp;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          return item->pointer;
        }
      }
    }
  }

  return NULL;
}

void link_hashmap_str_remove(link_hashmap_t *hash, const char *key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = link_hashmap_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;
  int cmp;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          link_hlist_remove(&item->node);
          link_pfree(item->pointer);
          link_pfree(item->key);
          link_pfree(item);
          hash->items--;
          return;
        }
      }
    }
  }
}

void link_hashmap_int_set(link_hashmap_t *hash, size_t key, void *pointer) {
  if (hash->items >= hash->max_items) {
    link_hashmap_rehash(hash);
  }

  size_t index = link_hashmap_slot(key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->hash == key) {
        link_pfree(item->pointer);
        item->pointer = pointer;
        return;
      }
    }
  }

  item = link_palloc(sizeof(link_hashmap_item_t));
  item->pointer = pointer;
  item->hash = key;
  link_hlist_insert_head(&item->node, slot);
  hash->items++;
}

void *link_hashmap_int_get(link_hashmap_t *hash, size_t key) {
  size_t index = link_hashmap_slot(key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->hash == key) {
        return item->pointer;
      }
    }
  }

  return NULL;
}

void link_hashmap_int_remove(link_hashmap_t *hash, size_t key) {
  size_t index = link_hashmap_slot(key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->hash == key) {
        link_hlist_remove(&item->node);
        link_pfree(item->pointer);
        link_pfree(item);
        hash->items--;
        return;
      }
    }
  }
}
