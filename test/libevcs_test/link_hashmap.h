/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LINK_HASHMAP_H
#define LINK_HASHMAP_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "link_list.h"

#define LINK_2_SHIFT             1
#define LINK_4_SHIFT             2
#define LINK_8_SHIFT             3
#define LINK_16_SHIFT            4
#define LINK_32_SHIFT            5
#define LINK_64_SHIFT            6
#define LINK_128_SHIFT           7
#define LINK_256_SHIFT           8
#define LINK_512_SHIFT           9
#define LINK_1K_SHIFT            10
#define LINK_2K_SHIFT            11
#define LINK_4K_SHIFT            12
#define LINK_8K_SHIFT            13
#define LINK_16K_SHIFT           14
#define LINK_32K_SHIFT           15
#define LINK_64K_SHIFT           16
#define LINK_128K_SHIFT          17
#define LINK_256K_SHIFT          18
#define LINK_512K_SHIFT          19
#define LINK_1M_SHIFT            20

#define link_malloc     malloc
#define link_palloc     malloc
#define link_free       free
#define link_pfree      free

#define link_lower(c)     (unsigned char) (c | 0x20)
#define link_upper(c)     (unsigned char) (c & ~0x20)

#define link_hash         link_hash_DJB
#define link_hash_lower   link_hash_DJB_lower
#define link_hash_slot    link_hash_slot64

static inline size_t link_hash_DJB(const char *str, size_t n) {
  size_t hash = 5381;

  for (size_t i = 0; i < n; i++) {
    hash += (hash << 5) + str[i];
  }

  return hash;
}

static inline size_t link_hash_DJB_lower(const char *str, size_t n) {
  size_t hash = 5381;
  
  for (size_t i = 0; i < n; i++) {
    hash += (hash << 5) + link_lower(str[i]);
  }

  return hash;
}

#define LINK_GOLDEN_RATIO_PRIME64       11400714819323198485UL

/* if slot size is 1024, used_bits = 10
 * bits = 64 - used_bits;
 * linux kernel hash.h is better
 */
static inline size_t link_hash_slot64(size_t value, size_t bits) {
  return (value * LINK_GOLDEN_RATIO_PRIME64) >> (64 - bits);
}

typedef int (*link_hash_strcmp_fn)(const char *s1, const char *s2, size_t n);
typedef size_t (*link_hash_fn)(const char *s, size_t n);

typedef struct {
  link_hlist_head_t     *slots;
  size_t                max_items;
  size_t                items;
  size_t                bits;
  link_hash_strcmp_fn   strcmp;
  link_hash_fn          hash;
} link_hashmap_t;

typedef struct {
  link_hlist_node_t   node;
  void                *pointer;
  size_t              hash;
  char                *key;
  size_t              keylen;
  intptr_t            value;
} link_hashmap_item_t;

link_hashmap_t *link_hashmap_create(size_t bits, link_hash_strcmp_fn strcmp_fn, link_hash_fn hash_fn);
void link_hashmap_destroy(link_hashmap_t *hash);

void link_hashmap_set_pointer(link_hashmap_t *hash, char *key, size_t n, void *pointer);
void *link_hashmap_get_pointer(link_hashmap_t *hash, const char *key, size_t n);
void link_hashmap_remove_pointer(link_hashmap_t *hash, const char *key, size_t n);

void link_hashmap_set_value(link_hashmap_t *hash, char *key, size_t n, intptr_t value);
int link_hashmap_get_value(link_hashmap_t *hash, const char *key, size_t n, intptr_t *value);
void link_hashmap_remove_value(link_hashmap_t *hash, const char *key, size_t n);

#endif /* LINK_HASHMAP_H */
