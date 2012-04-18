#ifndef SPARSE_LIST_H
#define SPARSE_LIST_H

typedef struct {
  int k;
  int *col;
  double *val;
} sparse_list;

sparse_list *sparse_list_parse(const char *line);

#ifdef DEBUG
void sparse_list_print(const sparse_list *l);
#endif

void sparse_list_free(sparse_list *l);

double sparse_list_dot(const sparse_list *a, const sparse_list *b);

void sparse_list_add_vector(double *w, const sparse_list *a, double t);

double sparse_list_dot_vector(const sparse_list *a, const double *b, int bn);

#endif /* sparse_list.h */
