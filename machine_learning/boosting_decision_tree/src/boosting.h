#ifndef BOOSTING_TABLE_H
#define BOOSTING_TABLE_H

#include "hash_table.h"

typedef struct boost boost;

boost *boost_train(int attributes, int classes, hash_table *x_table, int y[], int train_size, int iteration_times);
int boost_test(boost *bt, double x[]);
void boost_free(boost *bt);

#endif
