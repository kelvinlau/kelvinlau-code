#include "boosting.h"
#include "decision_tree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct boost {
  int size;
  dtree **weak_learners;
  double *alpha;
  int classes;
};

#define TRAIN_SIZE 2048
#define CLASSES    16

static double weight[TRAIN_SIZE];
static double distribution[CLASSES];
static int mask[TRAIN_SIZE];
static double bagging_possibility;

boost *boost_new(int size, int classes) {
  boost *bt = malloc(sizeof(boost));
  bt->size = size;
  bt->classes = classes;
  bt->weak_learners = malloc(sizeof(dtree *) * size);
  bt->alpha = malloc(sizeof(double) * size);
  return bt;
}

boost *boost_train(int attributes, int classes, hash_table *x, int y[], int train_size, int iterations) {
  int i, t;
  double e, a, total;
  boost *bt = boost_new(iterations, classes);

  bagging_possibility = (1.0 / iterations + 1.0) / 2.0;

  // init weight
  for (i = 0; i < train_size; i++)
    weight[i] = 1.0 / train_size;

  for (t = 0; t < iterations; t++) {

    printf("boosting iteration %d...\r", t + 1);
    fflush(stdout);

    // bagging
    for (i = 0; i < train_size; i++)
      mask[i] = (double) rand() / RAND_MAX < bagging_possibility;

    // train weak learner
    bt->weak_learners[t] = dt_train_subset(attributes, classes, x, y, weight, train_size, mask);

    // calculate error rate and importance of the weak learner
    e = 0;
    for (i = 0; i < train_size; i++)
      if (dt_test_ht(bt->weak_learners[t], x, i) != y[i])
        e += weight[i];

    if (e > .5) {
      puts("ooops");
      bt->size = t;
      break;
    }

    bt->alpha[t] = a = log((1 - e) / e) / 2;

    // update weight
    for (i = 0; i < train_size; i++) {
      if (dt_test_ht(bt->weak_learners[t], x, i) != y[i])
        weight[i] *= exp(a);
      else
        weight[i] /= exp(a);
      if (weight[i] > 20.0 / train_size)
        weight[i] = 20.0 / train_size;
    }

    // normalize weight
    total = 0;
    for (i = 0; i < train_size; i++)
      total += weight[i];

    for (i = 0; i < train_size; i++)
      weight[i] /= total;
  }
  puts("");
  return bt;
}

int boost_test(boost *bt, double x[]) {
  int i, o, bi;

  memset(distribution, 0, sizeof(distribution[0]) * bt->classes);
  for (i = 0; i < bt->size; i++) {
    o = dt_test(bt->weak_learners[i], x);
    distribution[o] += bt->alpha[i];
  }

  bi = 0;
  for (i = 1; i < bt->classes; i++)
    if (distribution[i] > distribution[bi])
      bi = i;
  return bi;
}

void boost_free(boost *bt) {
  int i;
  for (i = 0; i < bt->size; i++)
    dt_free(bt->weak_learners[i]);
  free(bt->weak_learners);
  free(bt->alpha);
  free(bt);
}
