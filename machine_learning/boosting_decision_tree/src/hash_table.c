#include "hash_table.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

hash_table *hash_table_new(int size, val_t default_value) {
  hash_table *ht = malloc(sizeof(hash_table));
  ht->size = size;
  ht->key = malloc(sizeof(key_t) * size);
  ht->val = malloc(sizeof(val_t) * size);
  ht->hash_key = malloc(sizeof(int) * size);
  ht->default_value = default_value;
  memset(ht->hash_key, 0xff, sizeof(int) * size);
  return ht;
}

void hash_table_free(hash_table *ht) {
  free(ht->key);
  free(ht->val);
  free(ht->hash_key);
  free(ht);
}

static inline int hash(key_t key) {
  return (key * key) & 0x7fffffff;
}

int hash_insert(hash_table *ht, key_t key1, key_t key2, val_t val) {
  key_t key = (key1 << 16) | key2;
  int hash_key = hash(key);
  int t = hash_key % ht->size;
  int k = 1;
  
  while (1) {
    if (ht->hash_key[t] == -1) {
      ht->hash_key[t] = hash_key;
      ht->key[t] = key;
      ht->val[t] = val;
      return 1;
    }
    t = (t + k) % ht->size;
    k = (k + 2) % ht->size;
  }
}

val_t hash_find(hash_table *ht, key_t key1, key_t key2) {
  key_t key = (key1 << 16) | key2;
  int hash_key = hash(key);
  int t = hash_key % ht->size;
  int k = 1;
  
  while (1) {
    if (ht->hash_key[t] == hash_key && ht->key[t] == key) {
      return ht->val[t];
    }
    if (ht->hash_key[t] == -1)
      return ht->default_value;
    t = (t + k) % ht->size;
    k = (k + 2) % ht->size;
  }
}
