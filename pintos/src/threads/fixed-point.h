#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#define FIX_Q 14
#define FIX(n) ((n) << FIX_Q)
#define FIX_INT(x) ((x) >> FIX_Q)
#define FIX_MUL(x, y) ((int64_t)(x) * (y) >> FIX_Q)
#define FIX_DIV(x, y) (((int64_t)(x) << FIX_Q) / (y))

#endif /* threads/fixed-point.h */
