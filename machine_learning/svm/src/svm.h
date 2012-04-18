#ifndef SVM_H
#define SVM_H

#include "sparse_list.h"

/************* kernels ***************/

typedef double (*kernel_func)(const sparse_list *, const sparse_list *);

double kernel_linear     (const sparse_list *a, const sparse_list *b);
double kernel_rbf        (const sparse_list *a, const sparse_list *b);
double kernel_polynomial (const sparse_list *a, const sparse_list *b);

/************* parameters ***************/

struct svm_model {
  kernel_func kernel;
  double C;
  double eps;
  double poly_degree;
  double poly_coef;
  double rbf_gamma;
  size_t cache_size;
};

const struct svm_model DEFAULT_SVM_MODEL;

/************* public functions ***************/

void svm_init(const struct svm_model *model);

void svm_train(FILE *f);

void svm_test(FILE *f);

double svm_output(const sparse_list *x);

void svm_clean(void);

#endif /* svm.h */
