#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svm.h"

#ifdef DEBUG
#include <assert.h>
#endif

#define BUF_SIZE            100000

/************* utils ***************/

static char buf[BUF_SIZE];

static inline int sign(double x) {
  return x > 0 ? +1 : -1;
}

static inline double max(double x, double y) {
  return x > y ? x : y;
}

static inline double min(double x, double y) {
  return x < y ? x : y;
}

static inline double frand(double a, double b) {
  return a + (double)rand() / RAND_MAX * (b - a);
}

#ifdef DEBUG
static void print_vector(double a[], int n) {
  int i;
  for (i = 0; i < n; i++)
    printf(" %+7.3lf", a[i]);
  puts("");
}
#endif

/************* kernels ***************/

double kernel_linear(const sparse_list *a, const sparse_list *b) {
  return sparse_list_dot(a, b);
}

static double rbf_gamma;

double kernel_rbf(const sparse_list *a, const sparse_list *b) {
  return exp(-rbf_gamma * sparse_list_dot(a, b));
}

static double poly_degree;
static double poly_coef;

double kernel_polynomial(const sparse_list *a, const sparse_list *b) {
  return pow(sparse_list_dot(a, b) + poly_coef, poly_degree);
}

/************* svm ***************/

static double eps;
static double C;
static kernel_func kernel;

static int n;              /* # training samples */
static int dim;            /* dimension */
static sparse_list **x;    /* x */
static int *y;             /* y */
static double *a, b;       /* alpha[], b */
static double *w;          /* w[] for linear kernel */
static int *sv, *svr, nsv; /* support vectors indexes */
static int *bv, *bvr, nbv; /* non bound vectors indexes */
static double *out;
static double **kern;
static double *kern_self;
static size_t cache_size;
static int cache_ts, cache_hit, cache_op;
static int full_cached;

const struct svm_model DEFAULT_SVM_MODEL = {
  .kernel      = kernel_linear,
  .C           = 1.0,
  .eps         = 1e-3,
  .poly_degree = 2.0,
  .poly_coef   = 0.0,
  .rbf_gamma   = 0.01,
  .cache_size  = (100 << 20),
};

/* 
 * initialize the svm, set parameters (& kernel type) here 
 * put parameters in struct svm_model
 * set model to NULL to use default parameters
 */
void svm_init(const struct svm_model *model) {
  if (model == NULL)
    model = &DEFAULT_SVM_MODEL;

  kernel = model->kernel;
  C = model->C;
  poly_degree = model->poly_degree;
  poly_coef = model->poly_coef;
  rbf_gamma = model->rbf_gamma;
  eps = model->eps;
  cache_size = model->cache_size;
}

static int non_bound(double a) {
  return eps < a && a < C - eps;
}

struct cache_elem {
  int i, j, ts;
  double v;
} *cache_table;

static void cache_init(void) {
  int i, j;
  if (cache_size >= n * n)
    full_cached = 1;
  if (full_cached) {
    puts("calculating kernel table...");
    kern = malloc(sizeof(double *) * n);
    for (i = 0; i < n; i++)
      kern[i] = malloc(sizeof(double) * n);
    for (i = 0; i < n; i++)
      for (j = i; j < n; j++)
        kern[i][j] = kern[j][i] = kernel(x[i], x[j]);
  } else {
    kern_self = malloc(sizeof(double) * n);
    for (i = 0; i < n; i++)
      kern_self[i] = kernel(x[i], x[i]);
    cache_table = malloc(sizeof(struct cache_elem) * cache_size);
    for (i = 0; i < cache_size; i++)
      cache_table[i].ts = 0;
    cache_hit = cache_op = 0;
    cache_ts = 0;
  }
}

static void cache_free(void) {
  int i;
  if (full_cached) {
    for (i = 0; i < n; i++)
      free(kern[i]);
    free(kern);
  } else {
    free(kern_self);
    free(cache_table);
  }
}

static inline int hash(int i, int j) {
  return (((i * 0xf816337f) ^ (j * 0x407f661a) ^ 
        (i * j * 0x12345678)) & 0x7fffffff) % cache_size;
}

static double cached_kernel(int i, int j) {
  const int k = 3;
  int z, t;
  struct cache_elem *e, *f;

  if (full_cached)
    return kern[i][j];

  if (i == j)
    return kern_self[i];

  if (i > j) {
    t = i;
    i = j;
    j = t;
  }

  cache_op++;
  t = hash(i, j);
  f = NULL;
  for (z = 0; z < k; z++) {
    e = &cache_table[(t + z) % cache_size];
    if (e->i == i && e->j == j) {
      e->ts = ++cache_ts;
      cache_hit++;
      return e->v;
    }
    if (!e->ts) {
      f = e;
      break;
    }
    if (!f || e->ts < f->ts) {
      f = e;
    }
  }
  f->i = i;
  f->j = j;
  f->ts = ++cache_ts;
  f->v = kernel(x[i], x[j]);
  return f->v;
}

static double svm_output_xi(int i) {
  int z, k;
  double e;

  if (non_bound(a[i]))
    return out[i];

  if (kernel == kernel_linear)
    return sparse_list_dot_vector(x[i], w, dim) - b;

  e = -b;
  for (z = 0; z < nsv; z++) {
    k = sv[z];
    e += cached_kernel(i, k) * a[k] * y[k];
  }
  return e;
}

static int num_samples(FILE *f) {
  int samples = 0;
  rewind(f);
  while (fgets(buf, BUF_SIZE, f)) {
    if (buf[0] == '#') continue;
    samples++;
  }
  rewind(f);
  return samples;
}

static void read_sample(const char *line, int *y, sparse_list **x) {
  int k;
  sscanf(buf, "%d%n", y, &k);
  *x = sparse_list_parse(buf + k);
}

/*
 * train a svm model using SMO algorithm from a data file
 */
void svm_train(FILE *f) {
  int i, j, k, z, ti, tj, bz, sti, stj;
  double ai, aj, ei, ej, ek, dem, li, hi, bi, bj;
  double lj, hj, fj, fi, lobj, hobj, oldb, kii, kjj, kij;
  int loop, s, vi, vj, change, all, iterations = 0;

  /* alloc memory */
  n   = num_samples(f);
  x   = malloc(n * sizeof(*x));
  y   = malloc(n * sizeof(*y));
  a   = malloc(n * sizeof(*a));
  sv  = malloc(n * sizeof(*sv));
  svr = malloc(n * sizeof(*svr));
  bv  = malloc(n * sizeof(*bv));
  bvr = malloc(n * sizeof(*bvr));
  out = malloc(n * sizeof(*out));

  /* input */
  puts("reading training data...");

  n = 0;
  while (fgets(buf, BUF_SIZE, f)) {
    if (buf[0] == '#') continue;
    read_sample(buf, &y[n], &x[n]);
    n++;
  }
  printf("training data set size: %d\n", n);

  /* kernel table */
  cache_init();

  /* initialize a[], b, sv[], bv[], maybe w[] */
  for (i = 0; i < n; i++) {
    a[i] = 0.0;
    svr[i] = -1;
    bvr[i] = -1;
  }
  if (kernel == kernel_linear) {
    dim = 0;
    for (i = 0; i < n; i++)
      if (dim < x[i]->col[x[i]->k - 1])
        dim = x[i]->col[x[i]->k - 1];
    dim++;
    w = malloc(sizeof(double) * dim);
    for (i = 0; i < dim; i++)
      w[i] = 0.0;
  }
  b = 0.0;
  nsv = nbv = 0;

  /* start training */
  puts("training...");
  for (loop = all = 1; all || change; loop++) {
#ifdef DEBUG
    if (loop > 1)
      printf("\033[4A");
    printf("------- loop %d, all = %d ------------\n", loop, all);
#endif
    change = 0;
    /* choose i that violates KKT conditions */
    for (ti = 0, sti = rand() % n; ti < (all ? n : nbv); ti++) {
      i = (all ? (sti + ti) % n : bv[ti]);
      ei = svm_output_xi(i) - y[i];
      if ((ei * y[i] < -eps && a[i] < C - eps) || 
          (ei * y[i] > eps && a[i] > eps)) {
        /* choose j with max |ei - ej| */
        bz = -1;
        for (z = 0; z < nbv; z++) {
          k = bv[z];
          ek = svm_output_xi(k) - y[k];
          if (bz == -1 || fabs(ek - ei) > fabs(ej - ei)) {
            bz = z;
            ej = ek;
          }
        }
        for (tj = 0, stj = rand() % n; tj < nbv + n; tj++) {
          j = (tj < nbv ? bv[(bz + tj) % nbv] : (stj + tj) % n);
          if (j == i) continue;
          if (tj >= nbv && bvr[j] != -1) continue;
          /* update a[i], a[j] */
          s = y[i] * y[j];
          ej = svm_output_xi(j) - y[j];
          kii = cached_kernel(i, i);
          kjj = cached_kernel(j, j);
          kij = cached_kernel(i, j);
          dem = kii + kjj - 2.0 * kij;
          li = y[i] != y[j] ? max(0.0, a[i] - a[j]) : max(0.0, a[i] + a[j] - C);
          hi = y[i] != y[j] ? min(C, C + a[i] - a[j]) : min(C, a[i] + a[j]);
          if (li > hi - eps) continue;

          if (dem > eps) {
            ai = a[i] + y[i] * (ej - ei) / dem;
            ai = max(min(ai, hi), li);
          } else {
            lj = a[j] + s * (a[i] - li);
            hj = a[j] + s * (a[i] - hi);
            fj = y[j] * (ej + b) - a[j] * kjj - s * a[i] * kij;
            fi = y[i] * (ei + b) - a[i] * kii - s * a[j] * kij;
            lobj = -0.5 * lj * lj * kjj - 0.5 * li * li * kii - 
              s * li * lj * kij - lj * fj - li * fi;
            hobj = -0.5 * hj * hj * kjj - 0.5 * hi * hi * kii - 
              s * hi * hj * kij - hj * fj - hi * fi;
            if (lobj > hobj + eps)
              ai = li;
            else if (lobj < hobj - eps)
              ai = hi;
            else
              ai = a[i];
          }

          aj = a[j] + s * (a[i] - ai);
          aj = max(0.0, aj);

          if (fabs(a[i] - ai) < eps / 10.0) continue;
          /* if (fabs(a[i] - ai) < eps * (a[i] + ai + eps)) continue; */

          /* update b */
          oldb = b;
          bj = ej + y[j] * (aj - a[j]) * kjj + y[i] * (ai - a[i]) * kij + b;
          bi = ei + y[j] * (aj - a[j]) * kij + y[i] * (ai - a[i]) * kii + b;
          vi = non_bound(a[i]);
          vj = non_bound(a[j]);
          if (vi ^ vj)
            b = vi ? bi : bj;
          else
            b = (bi + bj) / 2.0;

          /* update support vector sv[] */
          if (a[i] < eps && ai > eps) 
            svr[sv[nsv] = i] = nsv, nsv++;
          if (a[j] < eps && aj > eps) 
            svr[sv[nsv] = j] = nsv, nsv++;
          if (a[i] > eps && ai < eps) 
            k = svr[i], svr[sv[k] = sv[--nsv]] = k, svr[i] = -1;
          if (a[j] > eps && aj < eps) 
            k = svr[j], svr[sv[k] = sv[--nsv]] = k, svr[j] = -1;

          /* update non bound vector bv[] */
          if (!non_bound(a[i]) && non_bound(ai)) 
            bvr[bv[nbv] = i] = nbv, nbv++;
          if (!non_bound(a[j]) && non_bound(aj)) 
            bvr[bv[nbv] = j] = nbv, nbv++;
          if (non_bound(a[i]) && !non_bound(ai)) 
            k = bvr[i], bvr[bv[k] = bv[--nbv]] = k, bvr[i] = -1;
          if (non_bound(a[j]) && !non_bound(aj)) 
            k = bvr[j], bvr[bv[k] = bv[--nbv]] = k, bvr[j] = -1;

          /* update cache */
          out[i] = ei + y[i];
          out[j] = ej + y[j];
          for (z = 0; z < nbv; z++) {
            k = bv[z];
            out[k] += y[i] * (ai - a[i]) * cached_kernel(i, k) + 
                      y[j] * (aj - a[j]) * cached_kernel(j, k) + oldb - b;
          }

          /* update w[] if kernel is linear */
          if (kernel == kernel_linear) {
            sparse_list_add_vector(w, x[i], y[i] * (ai - a[i]));
            sparse_list_add_vector(w, x[j], y[j] * (aj - a[j]));
          }

          /* update a[] */
          a[i] = ai;
          a[j] = aj;

#ifndef DEBUG
          printf("iteration %d... support vectors = %d    \r", iterations, nsv);
          fflush(stdout);
#endif
          change = 1;
          iterations++;
          break;
        }
      }
    }
    if (all)
      all = 0;
    else if (!change)
      all = 1;
#ifdef DEBUG
    for (k = 0; k < nbv; k++)
      assert(non_bound(a[bv[k]]));
    for (i = k = 0 ; i < n; i++) {
      ei = svm_output_xi(i) - y[i];
      if ((ei * y[i] < -eps && a[i] < C - eps) || 
          (ei * y[i] > eps && a[i] > eps))
        k++;
    }
    printf("violates kkt: %d\033[K\n", k);
    printf("nsv: %d\033[K\n", nsv);
    printf("iteration: %d\033[K\n", iterations);
#endif
  }
  puts("");
  puts("training completed");

  /* statistics */
#ifdef DEBUG
  getchar();

  print_vector(a, n);
  printf("b = %lf\n", b);
  getchar();

  puts("violates kkt:");
  for (i = k = 0; i < n; i++) {
    ei = svm_output_xi(i) - y[i];
    if ((ei * y[i] < -eps && a[i] < C - eps) || 
        (ei * y[i] > eps && a[i] > eps)) {
      printf("i=%d: a=%.2lf y=%+d out=%+lf\n", i, a[i], y[i], svm_output_xi(i));
      k++;
    }
  }
  if (!k)
    puts("nothing violates kkt");
  getchar();

  puts("sv:");
  for (i = k = 0; i < n; i++)
    if (a[i] > eps) {
      k++;
      if (k < 20)
        printf("i=%d: a=%.2lf y=%+d out=%+lf\n", 
            i, a[i], y[i], svm_output_xi(i));
    }
  puts("...");
  getchar();
#endif

  printf("support vectors: %d\n", nsv);
  printf("outliers: %d\n", nsv - nbv);
  printf("cache hit rate: %.2lf%%\n", 
      (full_cached ? 100.0 : 100.0 * cache_hit / cache_op));

  /* clean up */
  free(bv);
  free(svr);
  free(bvr);
  free(out);
  cache_free();
}

/*
 * input a single sample x[]
 * return y
 * requires trained svm model
 */
double svm_output(const sparse_list *xt) {
  int z, k;
  double e;

  if (kernel == kernel_linear)
    return sparse_list_dot_vector(xt, w, dim);

  e = -b;
  for (z = 0; z < nsv; z++) {
    k = sv[z];
    e += y[k] * a[k] * kernel(x[k], xt);
  }
  return e;
}

/*
 * test the trained svm model, with a test data file, line by line, 
 * print correctness statistics
 */
void svm_test(FILE *f) {
  int yt, rev, test_total, test_pass;
  sparse_list *xt;

  puts("testing...");
  test_pass = test_total = 0;
  while (fgets(buf, BUF_SIZE, f)) {
    if (buf[0] == '#') continue;
    read_sample(buf, &yt, &xt);
    rev = sign(svm_output(xt));
    if (yt == rev) 
      test_pass++;
#ifdef DEBUG
    else if (test_total - test_pass < 10) {
      printf("%d: %+d vs %+d ", test_total, yt, rev);
      printf("%+.8lf ", svm_output(xt));
      puts("");
    }
#endif
    test_total++;
    sparse_list_free(xt);
  }
#ifdef DEBUG
  puts("...");
#endif
  printf("%d/%d (%.2lf%%) test data passed\n", 
      test_pass, test_total, 100.0 * test_pass / test_total);
}

/*
 * clean up the trained svm model
 */
void svm_clean(void) {
  int i;
  if (kernel == kernel_linear)
    free(w);
  for (i = 0; i < n; i++)
    sparse_list_free(x[i]);
  free(x);
  free(y);
  free(a);
  free(sv);
}
