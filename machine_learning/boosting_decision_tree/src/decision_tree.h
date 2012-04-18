#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include "hash_table.h"

typedef struct dtree dtree;

dtree *dt_train(int attributes, int classes, hash_table *x, int y[], double weight[], int train_size);
dtree *dt_train_subset(int attributes, int classes, hash_table *x, int y[], double weight[], int train_size, int mask[]);
int dt_test(dtree *u, double x[]);
int dt_test_ht(dtree *u, hash_table *x, int row);
void dt_free(dtree *u);

#endif
