#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash_table.h"
#include "boosting.h"

#define BUFFER_SIZE         10000
#define TRAIN_SIZE          2048
#define CLASSES             2
#define ATTRIBUTES          10000
#define DEFAULT_ITERATIONS  20

static char line[BUFFER_SIZE];
static int y[TRAIN_SIZE];
static double x[ATTRIBUTES];
static hash_table *x_table;
static int row, col;

static inline char sign(int x) {
  return x > 0 ? '+' : '-';
}

int main(int argc, char *argv[]) {
  int j, k, iterations;
  int ans, rev;
  double v;
  int correct, test;
  char *ptr;
  FILE *f, *g;
  boost *bt;

  // process arguments
  if (argc != 3 && argc != 4) {
    printf("usage: %s <train data> <test data> [iterations (default = %d)]\n", argv[0], DEFAULT_ITERATIONS);
    return 1;
  }

  f = fopen(argv[1], "r");
  if (!f) {
    printf("[error] cannot open file `%s'\n", argv[2]);
    return 1;
  }

  g = fopen(argv[2], "r");
  if (!g) {
    printf("[error] cannot open file `%s'\n", argv[2]);
    return 1;
  }

  iterations = DEFAULT_ITERATIONS;
  if (argc == 4)
    sscanf(argv[3], "%d", &iterations);

  // input data
  x_table = hash_table_new(100003, 0.0);
  row = col = 0;

  fgets(line, BUFFER_SIZE, f);
  while (fgets(line, BUFFER_SIZE, f)) {
    ptr = line;
    sscanf(line, "%d%n", &y[row], &k);
    y[row] = (y[row] == 1);
    ptr += k;

    while (EOF != sscanf(ptr, "%d:%lf%n", &j, &v, &k)) {
      ptr += k;
      hash_insert(x_table, row, j, v);
      if (col < j + 1)
        col = j + 1;
    }

    row++;
  }

  // train
  puts("training...");
  bt = boost_train(col, CLASSES, x_table, y, row, iterations);
  puts("train completed");

  // test
  puts("testing...");
  test = correct = 0;
  fgets(line, BUFFER_SIZE, g);
  while (fgets(line, BUFFER_SIZE, g)) {
    ptr = line;
    sscanf(line, "%d%n", &ans, &k);
    ptr += k;

    memset(x, 0, col * sizeof(double));
    while (EOF != sscanf(ptr, "%d:%lf%n", &j, &v, &k)) {
      ptr += k;
      x[j] = v;
    }

    rev = boost_test(bt, x);
    rev = rev ? +1 : -1;

    test++;
    if (rev == ans) correct++;
    if (rand() % 20 == 0) printf("%s  expected %c  recieved %c\n", ans == rev ? "pass" : "FAIL", sign(ans), sign(rev));
  }
  puts("...");
  printf("%d of %d tests passed (%.1lf%%)\n", correct, test, 100.0 * correct / test);

  // clean up
  boost_free(bt);
  hash_table_free(x_table);

  fclose(f);
  fclose(g);

  return 0;
}
