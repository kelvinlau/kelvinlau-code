#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define L 160

/* original data */
struct line {
  int x, y;
  char s[L];
};

void line_read(FILE * f, struct line *line) {
  fscanf(f, "%[^\n]%*[\n]", line->s);
  sscanf(line->s, "%d%*c%d", &line->x, &line->y);
}

void line_write(FILE * f, struct line *line) {
  fprintf(f, "%s\n", line->s);
}

/* extracted useful data, can greatly reduce I/O while sorting */
struct record {
  int x, y, p;
};

int record_cmp(const void *a_, const void *b_) {
  const struct record *a = a_, *b = b_;
  if (a->x - b->x)
    return a->x - b->x;
  return a->y - b->y;
}

#define K 130                    /* pages number */
#define P 8192                   /* page size */
#define S sizeof(struct record)  /* record size */
#define M (P / S)                /* records per page */

int k, db;
struct record buffer[K][M];
int st[K], ed[K], pt[K];
int heap[K], hn;

#define buf(i) buffer[i + i * db][pt[i]]

int min(int x, int y) {
  return x < y ? x : y;
}

void heap_push(int x) {
  int i, j, y;

  for (i = hn++; i; i = j) {
    j = (i - 1) >> 1;
    y = heap[j];
    if (record_cmp(&buf(x), &buf(y)) >= 0)
      break;
    heap[i] = y;
  }
  heap[i] = x;
}

int heap_pop() {
  int i, x, y, z, l, r, res;

  res = heap[0];
  x = heap[--hn];
  i = 0;
  while (1) {
    l = i + i + 1;
    r = i + i + 2;
    z = x;
    if (l < hn) {
      y = heap[l];
      if (record_cmp(&buf(y), &buf(z)) < 0)
        z = y;
    }
    if (r < hn) {
      y = heap[r];
      if (record_cmp(&buf(y), &buf(z)) < 0)
        z = y;
    }
    if (z == x)
      break;
    heap[i] = z;
    i = (z == heap[l] ? l : r);
  }
  heap[i] = x;

  return res;
}

void merge_sort(FILE * f, FILE * g, int in_place, int u, int v) {
  int pages, d, r, i, j, a, b;

  /* no pages */
  pages = (v - u + M - 1) / M;
  if (pages == 0)
    return;

  /* 1 pages */
  if (pages == 1) {
    if (!in_place) {
      fseek(f, u * S, SEEK_SET);
      fread(buffer[0], S, v - u, f);
      fwrite(buffer[0], S, v - u, g);
    }
    return;
  }

  /* merge sort recursively */
  d = pages / k;
  r = pages % k;
  for (i = 0, a = u; i < k; i++, a = b) {
    b = min(a + (d + (i < r)) * M, v);
    merge_sort(f, g, !in_place, a, b);
  }

  /* swap f, g if needed */
  if (!in_place) {
    FILE *t;
    t = f;
    f = g;
    g = t;
  }

  /* fill buffer */
  for (i = 0, a = u; i < k; i++, a = b) {
    b = min(a + (d + (i < r)) * M, v);

    st[i] = a;
    ed[i] = b;
    pt[i] = 0;

    fseek(g, a * S, SEEK_SET);
    fread(buffer[i], S, min(M, b - a), g);
  }

  /* initialize heap */
  hn = 0;
  for (i = 0; i < k; i++)
    if (st[i] < ed[i])
      heap_push(i);

  /* merge */
  d = 0;
  fseek(f, u * S, SEEK_SET);
  while (u < v) {
    /* copy smallest record to output buffer */
    i = heap_pop();
    buffer[k][d] = buf(i);

    /* adjust the input buffer */
    st[i]++;
    if (++pt[i] >= M) {
      pt[i] = 0;
      fseek(g, st[i] * S, SEEK_SET);
      fread(buffer[i], S, min(ed[i] - st[i], M), g);
    }
    /* adjust the heap */
    if (st[i] < ed[i])
      heap_push(i);

    /* adjust the output buffer */
    u++;
    if (++d >= M) {
      d = 0;
      fwrite(buffer[k], S, M, f);
    }
  }
  /* write back remaining data */
  fwrite(buffer[k], S, d, f);
}

struct arg {
  FILE *f;
  int u, v, i;
};

sem_t sem[K + 1];
pthread_mutex_t mutex[K + 1];
pthread_t tid[K + 1];
struct arg arg[K + 1];

void *parallel_write(void *arg_) {
  struct arg *arg = arg_;

  int u = arg->u;
  int v = arg->v;
  int d = 0;
  FILE *f = arg->f;

  fseek(f, u * S, SEEK_SET);

  while (u < v) {
    sem_wait(&sem[k]);
    fwrite(buffer[k + k + d], S, min(v - u, M), f);
    pthread_mutex_unlock(&mutex[k + k + d]);

    d = !d;
    u += min(v - u, M);
  }
}

void *parallel_read(void *arg_) {
  struct arg *arg = arg_;

  int u = arg->u;
  int v = arg->v;
  int i = arg->i;
  int d = 1;
  FILE *f = arg->f;

  while (u < v) {
    sem_wait(&sem[i]);

    pthread_mutex_lock(&mutex[K]);
    fseek(f, u * S, SEEK_SET);
    fread(buffer[i + i + d], S, min(v - u, M), f);
    pthread_mutex_unlock(&mutex[K]);

    pthread_mutex_unlock(&mutex[i + i + d]);

    d = !d;
    u += min(v - u, M);
  }

  pthread_detach(pthread_self());
}

void merge_sort_db(FILE * f, FILE * g, int in_place, int u, int v) {
  int pages, d, r, i, j, a, b;

  /* no pages */
  pages = (v - u + M - 1) / M;
  if (pages == 0)
    return;

  /* 1 pages */
  if (pages == 1) {
    if (!in_place) {
      fseek(f, u * S, SEEK_SET);
      fread(buffer[0], S, v - u, f);
      fwrite(buffer[0], S, v - u, g);
    }
    return;
  }

  /* merge sort recursively */
  d = pages / k;
  r = pages % k;
  for (i = 0, a = u; i < k; i++, a = b) {
    b = min(a + (d + (i < r)) * M, v);
    merge_sort_db(f, g, !in_place, a, b);
  }

  /* swap f, g if needed */
  if (!in_place) {
    FILE *t;
    t = f;
    f = g;
    g = t;
  }

  /* fill buffer */
  for (i = 0, a = u; i < k; i++, a = b) {
    b = min(a + (d + (i < r)) * M, v);

    st[i] = a;
    ed[i] = b;
    pt[i] = 0;

    fseek(g, a * S, SEEK_SET);
    fread(buffer[i + i], S, min(M, b - a), g);
  }

  /* initialize heap */
  hn = 0;
  for (i = 0; i < k; i++)
    if (st[i] < ed[i])
      heap_push(i);

  /* create thread to write */
  arg[k].f = f;
  arg[k].u = u;
  arg[k].v = v;
  if (pthread_create(&tid[k], 0, parallel_write, &arg[k]))
    puts("ooops!");

  /* create threads to read */
  for (i = 0; i < k; i++) {
    if (ed[i] - st[i] > M) {
      arg[i].f = g;
      arg[i].u = st[i] + M;
      arg[i].v = ed[i];
      arg[i].i = i;
      if (pthread_create(&tid[i], 0, parallel_read, &arg[i]))
        puts("ooops!");

      j = i + i + 1;
      pthread_mutex_lock(&mutex[j]);
      sem_post(&sem[i]);
    }
  }

  /* merge */
  d = 0;
  while (u < v) {
    /* copy smallest record to output buffer */
    i = heap_pop();
    buffer[k + k][d] = buf(i);

    /* adjust the input buffer */
    st[i]++;
    pt[i]++;
    if (pt[i] % M == 0) {
      /* wait for next half of buffer */
      if (st[i] < ed[i]) {
        j = i + i + pt[i] / M % 2;
        pthread_mutex_lock(&mutex[j]);
        pthread_mutex_unlock(&mutex[j]);
      }
      /* request next next half of buffer */
      a = min(st[i] + M, ed[i]);
      if (a < ed[i]) {
        j = i + i + pt[i] / M - 1;
        pthread_mutex_lock(&mutex[j]);
        sem_post(&sem[i]);
      }
      pt[i] %= M * 2;
    }
    /* adjust the heap */
    if (st[i] < ed[i])
      heap_push(i);

    /* adjust the output buffer */
    u++;
    d++;
    if (d % M == 0) {
      /* write back to file */
      pthread_mutex_lock(&mutex[k + k + d / M - 1]);
      sem_post(&sem[k]);
      d %= M * 2;

      /* wait for previous write back */
      if (u < v) {
        pthread_mutex_lock(&mutex[k + k + d / M]);
        pthread_mutex_unlock(&mutex[k + k + d / M]);
      }
    }
  }

  /* write back remaining data */
  if (d % M) {
    pthread_mutex_lock(&mutex[k + k + d / M]);
    sem_post(&sem[k]);
  }
  /* wait for output thread to exit */
  pthread_join(tid[k], 0);
}

/* format into single file */
void format(FILE * fo, FILE * f1, FILE * fi, int n) {
  int p, j;
  struct line line;

  rewind(fo);
  rewind(f1);

  while (n) {
    p = min(n, M * (k + 1) * (db + 1));
    fread(buffer[0], S, p, f1);
    for (j = 0; j < p; j++) {
      fseek(fi, buffer[0][j].p, SEEK_SET);
      line_read(fi, &line);
      line_write(fo, &line);
    }
    n -= p;
  }
}

/* format into 1kb pages */
void format_page(char *out, FILE * f1, FILE * fi, int n) {
  int p = 0, i, j, l, oft, num, ps;
  struct line line;
  char path[L];
  int *addr1;
  char *addr2;
  FILE *f;

  rewind(f1);

  sprintf(path, "%s.pages", out);
  mkdir(path, -1);
  chdir(path);

  for (i = 0; i < n; i = j) {
    ps = 4;
    oft = 0;
    num = 0;
    addr1 = (int *)((void *)buffer[1] + 1024 - 4);
    addr2 = (char *)buffer[1];

    for (j = i; j < n; j++) {
      if (j % M == 0) {
        fseek(f1, j * S, SEEK_SET);
        fread(buffer[0], S, min(M, n - j), f1);
      }

      fseek(fi, buffer[0][j % M].p, SEEK_SET);
      line_read(fi, &line);
      l = strlen(line.s);

      if (ps + l + 8 <= 1024) {
        ps += l + 8;

        *--addr1 = oft;
        *--addr1 = l;

        memcpy(addr2, line.s, l);
        addr2 += l;

        oft += l;
        num++;
      } else
        break;
    }

    *(int *)((void *)buffer[1] + 1024 - 4) = num;
    memset(addr2, 0, 1024 - ps);

    sprintf(path, "%s.%07d", out, p++);
    f = fopen(path, "w");
    fwrite(buffer[1], 1, 1024, f);
    fclose(f);
  }

  chdir("..");
}

/* check if the file sorted */
int check(FILE * f) {
  int i;
  struct line line;
  struct record *now, *pre;

  rewind(f);

  now = &buffer[0][0];
  pre = &buffer[0][1];
  for (i = 0; !feof(f); i++) {
    line_read(f, &line);
    now->x = line.x;
    now->y = line.y;
    if (i && record_cmp(pre, now) > 0) {
      printf("%d ", i);
      return 0;
    }
    *pre = *now;
  }
  return 1;
}

/* check if the pages sorted */
int check_page(char *out) {
  int i, ok = 1, num, p, len, oft, *addr, c = 0;
  struct record *now, *pre;
  char path[L];
  FILE *f;

  now = &buffer[0][0];
  pre = &buffer[0][1];

  sprintf(path, "%s.pages", out);
  chdir(path);

  pre->x = pre->y = 0;
  for (p = 0; ok; p++) {
    sprintf(path, "%s.%07d", out, p);
    f = fopen(path, "r");
    if (!f)
      break;

    fread(buffer[1], 1, 1024, f);

    addr = (int *)((void *)buffer[1] + 1024);
    num = *--addr;

    for (i = 0; i < num && ok; i++) {
      oft = *--addr;
      len = *--addr;
      memcpy(path, (char *)buffer[1] + oft, len);
      path[len] = 0;
      sscanf(path, "%d%*c%d", &now->x, &now->y);
      c++;
      if (record_cmp(pre, now) > 0) {
        printf("(%d %d %d)\n", p, i, c);
        ok = 0;
        break;
      }
      *pre = *now;
    }
    fclose(f);
  }
  chdir("..");
  return ok;
}

int main(int argc, char **argv) {
  int i, m, n, lim, chk, page_format;
  FILE *fi, *fo, *f1, *f2;

  /* print usage */
  if (argc < 3) {
    printf("usage: %s <infile> <outfile> [options]\n", argv[0]);
    printf("options:\n");
    printf("  -b <buffer-number>      set buffer number, default and maximum value is 130\n");
    printf("  -db                     enable double buffering\n");
    printf("  -p                      output 1kb pages (task 2)\n");
    printf("  -c                      check correctness when sort completed (debug)\n");
    printf("  -l <record-number>      sort first <record-number> records (debug)\n");
    return -1;
  }

  /* get arguments */
  k = K;
  lim = 0x7fffffff;
  db = chk = page_format = 0;
  for (i = 3; i < argc; i++) {
    if (!strcmp(argv[i], "-b")) {
      sscanf(argv[++i], "%d", &k);
      if (k > K) {
        fprintf(stderr, "error: too many buffer pages.");
        return -1;
      }
    } else if (!strcmp(argv[i], "-l")) {
      sscanf(argv[++i], "%d", &lim);
    } else if (!strcmp(argv[i], "-c")) {
      chk = 1;
    } else if (!strcmp(argv[i], "-db")) {
      db = 1;
    } else if (!strcmp(argv[i], "-p")) {
      page_format = 1;
    }
  }
  k = k / (db + 1) - 1;
  if (k < 2) {
    fprintf(stderr, "error: too few buffer pages.");
    return -1;
  }

  /* open files */
  fi = fopen(argv[1], "rb+");
  if (!fi) {
    fprintf(stderr, "error: file '%s' is not found.\n", argv[1]);
    return -1;
  }
  if (!page_format) {
    if (!strcmp(argv[1], argv[2])) {
      fprintf(stderr, "error: infile cannot be the same as outfile.\n");
      return -1;
    }
    fo = fopen(argv[2], "wb+");
  }
  f1 = tmpfile();
  f2 = tmpfile();

  /* confirm arguments */
  printf("buffer pages:     %d\n", (k + 1) * (db + 1));
  printf("double buffering: %s\n", db ? "yes" : "no");
  printf("output 1kb pages: %s\n", page_format ? "yes" : "no");

  /* start timer */
  time_t start = time(0);
  puts("sorting...");

  /* init mutex, semaphore */
  if (db) {
    for (i = 0; i <= K; i++) {
      pthread_mutex_init(&mutex[i], 0);
      sem_init(&sem[i], 0, 0);
    }
  }
  /* extract useful data, quick sort pages individualy   */
  struct line line;
  for (n = 0; !feof(fi) && n < lim;) {
    buffer[0][n % M].p = ftell(fi);
    line_read(fi, &line);
    buffer[0][n % M].x = line.x;
    buffer[0][n % M].y = line.y;

    n++;

    if (n % M == 0 || feof(fi) || n == lim) {
      int num = n % M ? n % M : M;
      qsort(buffer[0], num, S, record_cmp);
      fwrite(buffer[0], S, num, f1);
    }
  }

  /* merge sort */
  if (db)
    merge_sort_db(f1, f2, 1, 0, n);
  else
    merge_sort(f1, f2, 1, 0, n);

  /* format from useful data to real data */
  if (page_format)
    format_page(argv[2], f1, fi, n);
  else
    format(fo, f1, fi, n);

  /* stop timer */
  time_t end = time(0);
  int sec = (int)difftime(end, start);
  printf("completed, time elapsed: %d min %d sec\n", sec / 60, sec % 60);

  /* check correctness */
  if (chk) {
    printf("checking correctness: ");
    fflush(stdout);
    if (page_format ? check_page(argv[2]) : check(fo))
      puts("yes");
    else
      puts("no");
  }

  /* close files */
  fclose(f1);
  fclose(f2);
  fclose(fi);
  if (!page_format)
    fclose(fo);

  return 0;
}
