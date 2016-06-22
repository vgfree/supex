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
      index = link_hash_slot(item->hash, bits);
      link_hlist_insert_head(&item->node, &new_slots[index]);
    }
  }

  hash->slots = new_slots;
  link_free(old_slots);
}

link_hashmap_t *link_hashmap_create(size_t bits, link_hash_strcmp_fn strcmp_fn, link_hash_fn hash_fn) {
  link_hashmap_t *hash = link_malloc(sizeof(link_hashmap_t));
  if (hash == NULL) return NULL;

  hash->bits = bits;
  size_t size = 1 << bits;
  hash->max_items = size;
  hash->strcmp = strcmp_fn;
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

void link_hashmap_set_pointer(link_hashmap_t *hash, char *key, size_t n, void *pointer) {
  if (hash->items >= hash->max_items) {
    link_hashmap_rehash(hash);
  }

  size_t hash_key = hash->hash(key, n);
  size_t index = link_hash_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_hlist_entry(pos, link_hashmap_item_t, node);
      if (item->keylen == n) {
        if (hash->strcmp(item->key, key, n) == 0) {
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
  item->key = key;
  item->keylen = n;
  item->value = 0;
  link_hlist_insert_head(&item->node, slot);
  hash->items++;
}

void *link_hashmap_get_pointer(link_hashmap_t *hash, const char *key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = link_hash_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->keylen == n) {
        if (hash->strcmp(item->key, key, n) == 0) {
          return item->pointer;
        }
      }
    }
  }

  return NULL;
}

void link_hashmap_remove_pointer(link_hashmap_t *hash, const char *key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = link_hash_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->keylen == n) {
        if (hash->strcmp(item->key, key, n) == 0) {
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

void link_hashmap_set_value(link_hashmap_t *hash, char *key, size_t n, intptr_t value) {
  if (hash->items >= hash->max_items) {
    link_hashmap_rehash(hash);
  }

  size_t hash_key = hash->hash(key, n);
  size_t index = link_hash_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_hlist_entry(pos, link_hashmap_item_t, node);
      if (item->keylen == n) {
        if (hash->strcmp(item->key, key, n) == 0) {
          item->value = value;
          return;
        }
      }
    }
  }

  item = link_palloc(sizeof(link_hashmap_item_t));
  item->pointer = NULL;
  item->hash = hash_key;
  item->key = key;
  item->keylen = n;
  item->value = value;
  link_hlist_insert_head(&item->node, slot);
  hash->items++;
}

int link_hashmap_get_value(link_hashmap_t *hash, const char *key, size_t n, intptr_t *value) {
  size_t hash_key = hash->hash(key, n);
  size_t index = link_hash_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->keylen == n) {
        if (hash->strcmp(item->key, key, n) == 0) {
          *value = item->value;
          return 0;
        }
      }
    }
  }

  return -1;
}

void link_hashmap_remove_value(link_hashmap_t *hash, const char *key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = link_hash_slot(hash_key, hash->bits);
  link_hlist_head_t *slot = &hash->slots[index];
  link_hlist_node_t *pos;
  link_hashmap_item_t *item;

  if (!link_hlist_is_empty(slot)) {
    link_hlist_for_each(pos, slot) {
      item = link_list_entry(pos, link_hashmap_item_t, node);
      if (item->keylen == n) {
        if (hash->strcmp(item->key, key, n) == 0) {
          link_hlist_remove(&item->node);
          link_pfree(item->key);
          link_pfree(item);
          hash->items--;
          return;
        }
      }
    }
  }
}
