#include <stdio.h>
#include <string.h>
#include <algorithm>
using namespace std;

// C++11 only.
int bsearch(int n, function<bool(int)> f) {
  int l = 0, r = n;
  while (l < r) {
    int d = (l + r) >> 1;
    if (f(d)) {
      r = d;
    } else {
      l = d + 1;
    }
  }
  return r;
}

int main() {
  int a[] = {0, 1, 4, 9, 16, 25, 36};
  int n = 6;
  auto sqrt = [&](int x) {
    return bsearch(n, [&](int i) {
      return a[i] >= x;
    });
  };
  printf("%d\n", sqrt(16));
  return 0;
}
