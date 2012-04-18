#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "decision_tree.h"
#include "hash_table.h"

struct dtree {
  int sp_att;         /* split by this attribute; -1 if it's a leaf dtree */
  double threshold;   /* threshold value */
  dtree *l;           /* left dtree, attribute sp_att < threshold */
  dtree *r;           /* right dtree, attribute sp_att >= threshold */
  int type;           /* class type, for leaf dtree */
};

#define TRAIN_SIZE 2048  
#define CLASSES    16
#define CELLS      100000    
#define EPS        1e-4
#define INF        1e+100

static int attributes, classes;
static int row_idx[TRAIN_SIZE]; 
static double count[CLASSES], c1[CLASSES], c2[CLASSES];
static int cells[CELLS], cells2[CELLS];
static hash_table *x;
static int *y;
static double *weight;
static int global_max_type;

static void init(int a, int c, hash_table *tx, int ty[], double w[]) {
  attributes = a; 
  classes = c;
  x = tx;
  y = ty;
  weight = w;
}

static dtree *dtree_new(int sp_att, double threshold, dtree *l, dtree *r) {
  dtree *p = malloc(sizeof(dtree));
  p->sp_att = sp_att;
  p->threshold = threshold;
  p->l = l;
  p->r = r;
  return p;
}

static dtree *leaf_new(int type) {
  dtree *p = malloc(sizeof(dtree));
  p->sp_att = -1;
  p->type = type;
  p->l = p->r = NULL;
  return p;
}

static void swap(int *x, int *y) {
  int t;
  t = *x; *x = *y; *y = t;
}

static int decode_row(int c) {
  return c >> 16;
}

static int decode_col(int c) {
  return c & 0xffff;
}

static void make_count(int row_idx[], int n, double count[]) {
  int i;
  memset(count, 0, sizeof(count[0]) * classes);
  for (i = 0; i < n; i++)
    count[y[row_idx[i]]] += weight[row_idx[i]];
}

static int max_type(int row_idx[], int n) {
  int bi = global_max_type, i;
  double b = 0;
  static double count[CLASSES];

  make_count(row_idx, n, count);
  for (i = 0; i < classes; i++)
    if (count[i] > b) b = count[i], bi = i;
  return bi;
}

static double cal_total(double count[]) {
  int i;
  double total = 0;
  for (i = 0; i < classes; i++)
    total += count[i];
  return total;
}

static double entropy(double count[]) {
  int i;
  double p, res, total;
  
  total = cal_total(count);

  res = 0;
  for (i = 0; i < classes; i++) if (count[i]) {
    p = count[i] / total;
    res -= p * log(p);
  }
  return res;
}

/** 
 * @brief     partition indexes according to split_attribute and threshold
 *            < threshold:  left part
 *            >= threshold: right part
 */
static void partition_idx(int row_idx[], int n, int sp_att, double threshold, int *n1p, int *n2p) {
  int i;
  *n2p = n;
  for (i = 0; i < n; )
    if (hash_find(x, row_idx[i], sp_att) >= threshold)
      swap(&row_idx[--n], &row_idx[i]);
    else
      i++;
  *n1p = n;
  *n2p -= n;
}

/** 
 * @brief    partition cells according to split_attribute and threshold, discard cells on sp_att column
 */
static void partition_cells(int cells[], int cn, int sp_att, double threshold, int *cn1p, int *cn2p) {
  int i, cn1, cn2;

  cn1 = cn2 = 0;
  for (i = 0; i < cn; i++)
    if (decode_col(cells[i]) != sp_att) {
      if (hash_find(x, decode_row(cells[i]), sp_att) < threshold)
        cn1++;
      else
        cn2++;
    }
  cn2 = cn1;
  cn1 = 0;
  for (i = 0; i < cn; i++)
    if (decode_col(cells[i]) != sp_att) {
      if (hash_find(x, decode_row(cells[i]), sp_att) < threshold)
        cells2[cn1++] = cells[i];
      else
        cells2[cn2++] = cells[i];
    }
  memcpy(cells, cells2, cn * sizeof(cells[0]));
  *cn1p = cn1;
  *cn2p = cn2 - cn1;
}

/** 
 * @brief                build the decision tree recursively
 * @param row_idx[], n   subset indexes of training set
 * @param cells[], cn    subset indexes of non zero cells
 * @return               the built dtree
 */
static dtree *build(int row_idx[], int n, int cells[], int cn) {
  int i, j, k, best_att, n1, n2, cn1, cn2, zeroes, non_zero_class;
  double total, total1, total2;
  double best_split, emin, e;
  dtree *l, *r;

  make_count(row_idx, n, count);

  // train subset is empty
  if (!n)
    return leaf_new(global_max_type);

  // all are zeroes in area
  if (!cn)
    return leaf_new(max_type(row_idx, n));

  // all data have same classes
  zeroes = 0;
  for (i = 0; i < classes; i++) {
    if (!count[i]) zeroes++;
    else non_zero_class = i;
  }
  if (zeroes == classes - 1) {
    return leaf_new(non_zero_class);
  }

  // find best split point
  emin = INF;
  total = cal_total(count);
  for (i = j = 0; i < cn; i = j) {

    for (; j < cn && decode_col(cells[i]) == decode_col(cells[j]); j++);

    memcpy(c1, count, sizeof(count[0]) * classes);
    memset(c2, 0, sizeof(count[0]) * classes);
    total1 = total;
    total2 = 0;
    for (k = i; k < j; k++) {
      c2[y[decode_row(cells[k])]] += weight[decode_row(cells[k])]; total2 += weight[decode_row(cells[k])];
      c1[y[decode_row(cells[k])]] -= weight[decode_row(cells[k])]; total1 -= weight[decode_row(cells[k])];
    }

    for (k = i; k < j; k++) {
      c1[y[decode_row(cells[k])]] += weight[decode_row(cells[k])]; total1 += weight[decode_row(cells[k])];
      c2[y[decode_row(cells[k])]] -= weight[decode_row(cells[k])]; total2 -= weight[decode_row(cells[k])];

      if (k + 1 == j || hash_find(x, decode_row(cells[k]), decode_col(cells[k])) != hash_find(x, decode_row(cells[k + 1]), decode_col(cells[k + 1]))) {
        e = total1 / total * entropy(c1) + total2 / total * entropy(c2);
        if (e < emin) {
          emin = e;
          best_att = decode_col(cells[k]);
          best_split = hash_find(x, decode_row(cells[k]), decode_col(cells[k]));
        }
      }
    }
  }

  // no gain
  if (entropy(count) - emin < EPS) {
    return leaf_new(max_type(row_idx, n));
  }

  //recursively build
  partition_idx(row_idx, n, best_att, best_split, &n1, &n2);
  partition_cells(cells, cn, best_att, best_split, &cn1, &cn2);
  l = build(row_idx, n1, cells, cn1);
  r = build(row_idx + n1, n2, cells + cn1, cn2);
  return dtree_new(best_att, best_split, l, r);
}

static inline int sign(double val) {
  return val < 0 ? -1 : val > 0;
}

static int cell_cmp(const void *a_, const void *b_) {
  int a = *(int *)a_;
  int b = *(int *)b_;
  int ia = decode_row(a), ja = decode_col(a);
  int ib = decode_row(b), jb = decode_col(b);
  if (ja != jb)
    return ja - jb;
  return sign(hash_find(x, ia, ja) - hash_find(x, ib, jb));
}

dtree *dt_train(int attributes, int classes, hash_table *x, int y[], double weight[], int train_size) {
  int i, cn;

  init(attributes, classes, x, y, weight);

  for (i = 0; i < train_size; i++) 
    row_idx[i] = i;
  for (i = cn = 0; i < x->size; i++)
    if (x->hash_key[i] != -1)
      cells[cn++] = x->key[i];
  qsort(cells, cn, sizeof(cells[0]), cell_cmp);
  global_max_type = max_type(row_idx, train_size);

  return build(row_idx, train_size, cells, cn);
}

dtree *dt_train_subset(int attributes, int classes, hash_table *x, int y[], double weight[], int train_size, int mask[]) {
  int i, cn, num;

  init(attributes, classes, x, y, weight);

  num = 0;
  for (i = 0; i < train_size; i++) 
    if (mask[i]) row_idx[num++] = i;
  for (i = cn = 0; i < x->size; i++)
    if (x->hash_key[i] != -1 && mask[decode_row(x->key[i])])
      cells[cn++] = x->key[i];
  qsort(cells, cn, sizeof(cells[0]), cell_cmp);
  global_max_type = max_type(row_idx, num);

  return build(row_idx, num, cells, cn);
}

int dt_test(dtree *u, double x[]) {
  while (u->sp_att != -1) {
    if (x[u->sp_att] < u->threshold)
      u = u->l;
    else
      u = u->r;
  }
  return u->type;
}

int dt_test_ht(dtree *u, hash_table *x, int row) {
  while (u->sp_att != -1) {
    if (hash_find(x, row, u->sp_att) < u->threshold)
      u = u->l;
    else
      u = u->r;
  }
  return u->type;
}

void dt_free(dtree *x) {
  if (!x) return;
  dt_free(x->l);
  dt_free(x->r);
  free(x);
}
