#include "peg_solitaire.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <queue>

// ============================================================================
// class PegSolitaire
// ============================================================================

const int PegSolitaire::kDx[] = { -1, 0, 1, 0 };
const int PegSolitaire::kDy[] = { 0, 1, 0, -1 };
const int PegSolitaire::kDx8[] = { -1, 0, 1, 0, -1, 1, -1, 1 };
const int PegSolitaire::kDy8[] = { 0, 1, 0, -1, 1, 1, -1, -1 };

bool PegSolitaire::OnBoard(int x, int y) {
  if (x < 0 || x >= kN) return 0;
  if (y < 0 || y >= kN) return 0;
  if (x >= kN / 2) x = kN - 1 - x;
  if (y >= kN / 2) y = kN - 1 - y;
  return x >= 2 || y >= 2;
}

bool PegSolitaire::OnCenter(int x, int y) {
  return x == kCenter && x == y;
}

int PegSolitaire::NearEdge(int x, int y) {
  return ((x == 0 || x == kN - 1) << 1) | (y == 0 || y == kN - 1);
}

bool PegSolitaire::CanMove(const State &s, int x, int y, int d) {
  int x1 = x + kDx[d], y1 = y + kDy[d];
  int x2 = x1 + kDx[d], y2 = y1 + kDy[d];
  if (!OnBoard(x, y)) return false;
  if (!OnBoard(x2, y2)) return false;
  if (!s.s[x][y]) return false;
  if (!s.s[x1][y1]) return false;
  if (s.s[x2][y2]) return false;
  if (!NearEdge(x, y) && NearEdge(x2, y2)) return false;
  if (s.pcs == 32) return d == 0;
  return true;
}

bool PegSolitaire::Move(const State &s, int x, int y, int d, State *t) {
  int x1 = x + kDx[d], y1 = y + kDy[d];
  int x2 = x1 + kDx[d], y2 = y1 + kDy[d];
  if (!CanMove(s, x, y, d)) return false;
  *t = s;
  t->s[x][y] = 0;
  t->s[x1][y1] = 0;
  t->s[x2][y2] = 1;
  t->AddStat(x1, y1, -1);
  return true;
}

bool PegSolitaire::CanUnmove(const State &s, int x, int y, int d) {
  int x1 = x + kDx[d], y1 = y + kDy[d];
  int x2 = x1 + kDx[d], y2 = y1 + kDy[d];
  if (!OnBoard(x, y)) return false;
  if (!OnBoard(x2, y2)) return false;
  if (!s.s[x][y]) return false;
  if (s.s[x1][y1]) return false;
  if (s.s[x2][y2]) return false;
  if (NearEdge(x, y) && !NearEdge(x2, y2)) return false;
  return true;
}

bool PegSolitaire::Unmove(const State &s, int x, int y, int d, State *t) {
  int x1 = x + kDx[d], y1 = y + kDy[d];
  int x2 = x1 + kDx[d], y2 = y1 + kDy[d];
  if (!CanUnmove(s, x, y, d)) return false;
  *t = s;
  t->s[x][y] = 0;
  t->s[x1][y1] = 1;
  t->s[x2][y2] = 1;
  t->AddStat(x1, y1, 1);
  return true;
}

int PegSolitaire::Heuristic(const State &s) {
  int h = 0;
  int num[] = { s.d, s.b, s.c, s.a };
  for (int i = 0; i < 4; i++)
    for (int j = i + 1; j < 4; j++)
      if ((i ^ j) != 3)
        h += abs(num[i] - num[j]);
  for (int x = 0; x < kN; x++)
    for (int y = 0; y < kN; y++)
      if (OnBoard(x, y) && s.s[x][y]) {
        int neighbours = 0;
        for (int d = 0; d < kDir8; d++) {
          int x1 = x + kDx8[d];
          int y1 = y + kDy8[d];
          if (OnBoard(x1, y1) && s.s[x1][y1])
            neighbours++;
        }
        if (!neighbours)
          h += 7;
      }
  return h;
}

void PegSolitaire::ReversedBFS() {
  std::queue<State> queue;
  State end, u, v;

  end.InitEnd();
  queue.push(end);
  set2_.insert(end);

  while (!queue.empty()) {
    u = queue.front();
    queue.pop();

    if (u.pcs >= limit_)
      break;

    for (int x = 0; x < kN; x++)
      for (int y = 0; y < kN; y++) {
        if (OnBoard(x, y) && u.s[x][y]) {
          for (int d = 0; d < kDir; d++) {
            if (Unmove(u, x, y, d, &v) && v.CanEnd()) {
              if (!set2_.count(v)) {
                queue.push(v);
                set2_.insert(v);
              }
            }
          }
        }
      }
  }
}

void PegSolitaire::Search(int limit) {
  limit_ = limit;

  set2_.clear();
  ReversedBFS();
  puts("Done ReversedBFS");

  State start;
  start.InitStart();
  set1_.clear();
  set1_.insert(start);

  if (Dfs(start)) {
    puts("Found it!");
  }
}

bool PegSolitaire::Dfs(const State &u) {
  if (u.pcs <= limit_)
    return set2_.count(u);

//  if (rand() % 100000 == 0)
//    u.Dump();

  State v;
  std::vector<std::pair<int, int> > candidates;
  for (int x = 0; x < kN; x++)
    for (int y = 0; y < kN; y++) {
      if (OnBoard(x, y) && u.s[x][y]) {
        for (int d = 0; d < kDir; d++) {
          if (Move(u, x, y, d, &v) && !set1_.count(v) && v.CanEnd()) {
            candidates.push_back(
                std::make_pair(Heuristic(v), x * kN * kDir + y * kDir + d));
          }
        }
      }
    }
  std::sort(candidates.begin(), candidates.end());
  for (int i = 0; i < candidates.size() && i < kCL; i++) {
    int c = candidates[i].second;
    int x = c / kDir / kN;
    int y = c / kDir % kN;
    int d = c % kDir;
    Move(u, x, y, d, &v);
    if (!set1_.count(v)) {
      set1_.insert(v);
      if (Dfs(v)) {
        v.Dump();
        return true;
      }
      //set1_.erase(v);
    }
  }
  return false;
}

// ============================================================================
// class PegSolitaire::State
// ============================================================================

void PegSolitaire::State::InitStart() {
  for (int x = 0; x < kN; x++)
    for (int y = 0; y < kN; y++)
      s[x][y] = OnBoard(x, y) && !OnCenter(x, y);
  a = 4;
  b = c = 8;
  d = 12;
  pcs = a + b + c + d;
}

void PegSolitaire::State::InitEnd() {
  for (int x = 0; x < kN; x++)
    for (int y = 0; y < kN; y++)
      s[x][y] = OnCenter(x, y);
  a = 1;
  b = c = d = 0;
  pcs = a + b + c + d;
}

bool PegSolitaire::State::IsEnd() const {
  return pcs == 1 && s[kCenter][kCenter];
}

bool PegSolitaire::State::CanEnd() const {
  int d1 = s[0][2] + s[0][4] + s[6][0] + s[6][2];
  int d2 = s[2][0] + s[4][0] + s[0][6] + s[2][6];
  return a > 0 && d1 <= c && d2 <= b;
}

void PegSolitaire::State::Dump() const {
  for (int x = 0; x < kN; x++) {
    for (int y = 0; y < kN; y++) {
      if (OnBoard(x, y)) {
        putchar(s[x][y] ? 'o' : '.');
      } else {
        putchar(' ');
      }
      putchar(' ');
    }
    puts("");
  }
  printf("%d %d %d %d = %d\n", a, b, c, d, pcs);
  puts("");
}

void PegSolitaire::State::AddStat(int x, int y, int delta) {
  x %= 2;
  y %= 2;
  switch ((x << 1) | y) {
    case 0:
      d += delta;
      break;
    case 1:
      b += delta;
      break;
    case 2:
      c += delta;
      break;
    case 3:
      a += delta;
      break;
  }
  pcs += delta;
}

// ============================================================================
// class PegSolitaire::StateHasher
// ============================================================================

size_t
PegSolitaire::StateHasher::operator()(const PegSolitaire::State &s) const {
  size_t res = 0;
  for (int x = 0; x < kN; x++)
    for (int y = 0; y < kN; y++)
      if (OnBoard(x, y))
        res = (res << 1) | s.s[x][y];
  return res;
}

// ============================================================================
// class PegSolitaire::StateEq
// ============================================================================

bool PegSolitaire::StateEq::operator()(const State &s, const State &t) const {
  for (int x = 0; x < kN; x++)
    for (int y = 0; y < kN; y++)
      if (OnBoard(x, y))
        if (s.s[x][y] != t.s[x][y])
          return false;
  return true;
}
