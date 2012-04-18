#include <stdio.h>
#include <stdlib.h>
#include "sparse_list.h"

sparse_list *sparse_list_parse(const char *line) {
  sparse_list *l;
  int offset;
  int c, k;
  double v;
  const char *p;

  k = 0;
  p = line;
  while (sscanf(p, "%d:%lf%n", &c, &v, &offset) != EOF) {
    k++;
    p += offset;
  }

  l = malloc(sizeof(sparse_list));
  l->k = k;
  l->col = malloc(sizeof(int) * k);
  l->val = malloc(sizeof(double) * k);

  k = 0;
  p = line;
  while (sscanf(p, "%d:%lf%n", &c, &v, &offset) != EOF) {
    l->col[k] = c;
    l->val[k++] = v;
    p += offset;
  }

  return l;
}

#ifdef DEBUG
void sparse_list_print(const sparse_list *l) {
  int i;
  for (i = 0; i < l->k; i++)
    printf("%d:%lf ", l->col[i], l->val[i]);
  puts("");
}
#endif

void sparse_list_free(sparse_list *l) {
  free(l->col);
  free(l->val);
  free(l);
}

double sparse_list_dot(const sparse_list *a, const sparse_list *b) {
  int i, j;
  double res = 0.0;

  for (i = j = 0; i < a->k && j < b->k; ) {
    if (a->col[i] == b->col[j])
      res += a->val[i++] * b->val[j++];
    else if (a->col[i] < b->col[j])
      i++;
    else 
      j++;
  }
  return res;
}

void sparse_list_add_vector(double *w, const sparse_list *a, double t) {
  int i;
  for (i = 0; i < a->k; i++)
    w[a->col[i]] += t * a->val[i];
}

double sparse_list_dot_vector(const sparse_list *a, const double *b, int bn) {
  int i;
  double res = 0.0;
  for (i = 0; i < a->k; i++)
    if (a->col[i] < bn)
      res += b[a->col[i]] * a->val[i];
  return res;
}
