#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef int key_t;
typedef double val_t;

typedef struct hash_table {
  int size;
  key_t *key;
  val_t *val;
  int *hash_key;
  val_t default_value;
} hash_table;

hash_table *hash_table_new(int size, val_t default_value);
void hash_table_free(hash_table *ht);

int hash_insert(hash_table *ht, key_t key1, key_t key2, val_t value);
val_t hash_find(hash_table *ht, key_t key1, key_t key2);

#endif
