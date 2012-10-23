#ifndef PEG_SOLITAIRE_H_
#define PEG_SOLITAIRE_H_

#include "stdio.h"

#include <tr1/unordered_set>
#include <vector>

class PegSolitaire {
 public:
  static const int kN = 7;
  static const int kCenter = kN / 2;
  static const int kDir = 4;
  static const int kDx[];
  static const int kDy[];
  static const int kDir8 = 8;
  static const int kDx8[];
  static const int kDy8[];
  static const int kCL = 4;

  struct State {
    void InitStart();
    void InitEnd();
    bool IsEnd() const;
    bool CanEnd() const;
    void Dump() const;
    void AddStat(int x, int y, int delta);

    bool s[kN][kN];
    int a, b, c, d, pcs;
  };

  class StateHasher {
   public:
    size_t operator()(const State &s) const;
  };

  class StateEq {
   public:
    bool operator()(const State &s, const State &t) const;
  };

  void Search(int limit);

 private:
  typedef std::tr1::unordered_set<State, StateHasher, StateEq> StateSet;

  static bool OnBoard(int x, int y);
  static bool OnCenter(int x, int y);
  static int NearEdge(int x, int y);

  static bool CanMove(const State &s, int x, int y, int d);
  static bool Move(const State &s, int x, int y, int d, State *t);
  static bool CanUnmove(const State &s, int x, int y, int d);
  static bool Unmove(const State &s, int x, int y, int d, State *t);
  static int Heuristic(const State &s);

  void ReversedBfs();
  bool Dfs(const State &s);

  int limit_;
  StateSet set1_, set2_;
};


#endif  // PEG_SOLITAIRE_H_
