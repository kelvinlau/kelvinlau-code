#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "svm.h"

/*
 * train a svm model from a data file, then
 * test it from another data file
 */
int main(int argc, char **argv) {
  int i;
  double size_in_mb;
  char *val;
  FILE *f1, *f2;
  struct svm_model model = DEFAULT_SVM_MODEL;

  /* process args */
  f1 = f2 = NULL;
  for (i = 1; i < argc; i++) {

    if (!strcmp(argv[i], "-k")) {
      val = argv[++i];
      if (!strcmp(val, "linear") || !strcmp(val, "0")) {
        model.kernel = kernel_linear;

      } else if (!strcmp(val, "quadratic") || !strcmp(val, "1")) {
        model.kernel = kernel_polynomial;

      } else if (!strcmp(val, "rbf") || !strcmp(val, "2")) {
        model.kernel = kernel_rbf;

      } else {
        fprintf(stderr, "wrong kernel\n");
        return EXIT_FAILURE;
      }
    } else if (!strcmp(argv[i], "-c")) {
      sscanf(argv[++i], "%lf", &model.C);

    } else if (!strcmp(argv[i], "-e")) {
      sscanf(argv[++i], "%lf", &model.eps);

    } else if (!strcmp(argv[i], "-d")) {
      sscanf(argv[++i], "%lf", &model.poly_degree);

    } else if (!strcmp(argv[i], "-r")) {
      sscanf(argv[++i], "%lf", &model.poly_coef);

    } else if (!strcmp(argv[i], "-g")) {
      sscanf(argv[++i], "%lf", &model.rbf_gamma);

    } else if (!strcmp(argv[i], "-m")) {
      sscanf(argv[++i], "%lf", &size_in_mb);
      model.cache_size = (size_t)(size_in_mb * (1 << 20));

    } else if (!f1) {
      if (!(f1 = fopen(argv[i], "r"))) {
        fprintf(stderr, "cannot open file: %s\n", argv[i]);
        return EXIT_FAILURE;
      }
    } else if (!f2) {
      if (!(f2 = fopen(argv[i], "r"))) {
        fprintf(stderr, "cannot open file: %s\n", argv[i]);
        return EXIT_FAILURE;
      }
    }
  }
  if (!f1 || !f2) {
    fprintf(stderr, 
        "usage: %s [options] <train data file> <test data file>\n"
        "options:\n"
        "  -k kernel_type (default %d):\n"
        "     0: linear, u*v\n"
        "     1: polynomial, ((u*v+coef)^degree)\n"
        "     2: rbf, exp(-gamma*(u-v)^2)\n"
        "  -c cost_constant (default %.1lf)\n"
        "  -e epsilon (default %.g)\n"
        "  -d degree for polynomial kernel (default %.0lf)\n"
        "  -r coef for polynomial kernel (default %.2lf)\n"
        "  -g gamma for rbf kernel (default %.2lf)\n"
        "  -m kernel_cache_size in MB (default %d)\n",
        argv[0],
        0,
        DEFAULT_SVM_MODEL.C,
        DEFAULT_SVM_MODEL.eps,
        DEFAULT_SVM_MODEL.poly_degree,
        DEFAULT_SVM_MODEL.poly_coef,
        DEFAULT_SVM_MODEL.rbf_gamma,
        DEFAULT_SVM_MODEL.cache_size >> 20);
    return EXIT_FAILURE;
  }

  /* init */
  svm_init(&model);

  /* train */
  svm_train(f1);
  fclose(f1);

  /* test */
  svm_test(f2);
  fclose(f2);

  /* clean up */
  svm_clean();

  return EXIT_SUCCESS;
}
