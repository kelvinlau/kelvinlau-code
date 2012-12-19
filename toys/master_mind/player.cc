#include "player.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "game.h"

namespace master_mind {

// ----------------- Player ------------------

Player::Player() {}

Player::~Player() {}

// ----------------- GameAnalyst ------------------

class GameAnalyst {
 public:
  GameAnalyst() {
    Game::AppendAllStates(&pset_);
  }

  const vector<int>& PSet() const {
    return pset_;
  }

  void Update(int guess, int a, int b) {
    vector<int> pset;
    for (int i = 0; i < pset_.size(); ++i) {
      int a1, b1;
      Game::Compare(pset_[i], guess, &a1, &b1);
      if (a1 == a && b1 == b) {
        pset.push_back(pset_[i]);
      }
    }
    pset_.swap(pset);
  }

 private:
  vector<int> pset_;
};

// ----------------- HumanPlayer ------------------

HumanPlayer::HumanPlayer() : analyst_(new GameAnalyst) {
}

HumanPlayer::~HumanPlayer() {
  delete analyst_;
}

int HumanPlayer::Think() {
  char line[1 << 10];
  guess_ = 0;
  while (true) {
    printf(">> ");
    if (!fgets(line, sizeof line, stdin))
      break;
    line[strlen(line) - 1] = '\0';  // Strip newline.
    if (strcmp(line, "ls") == 0) {
      const vector<int>& pset = analyst_->PSet();
      for (int i = 0; i < pset.size(); ++i)
        printf("%04d    ", pset[i]);
      puts("");
    } else {
      sscanf(line, "%d", &guess_);
      break;
    }
  }
  return guess_;
}

void HumanPlayer::Info(int a, int b) {
  analyst_->Update(guess_, a, b);
}

// ----------------- SmartPlayer ------------------

SmartPlayer::SmartPlayer() : analyst_(new GameAnalyst) {
  moves_ = 0;
}

SmartPlayer::~SmartPlayer() {
  delete analyst_;
}

int SmartPlayer::Think() {
  if (++moves_ <= 2)
    return guess_ = Game::RandomState();

  const vector<int>& pset = analyst_->PSet();
  if (pset.size() == 1)
    return guess_ = pset[0];

  double emax = -1e100;
  for (int i = 0; i < pset.size(); ++i) {
    int g = pset[i];
    double e = DecisionEntropy(g);
    if (emax < e) {
      emax = e;
      guess_ = g;
    }
  }
  return guess_;
}

void SmartPlayer::Info(int a, int b) {
  analyst_->Update(guess_, a, b);
}

double SmartPlayer::DecisionEntropy(int g) {
  const vector<int>& pset = analyst_->PSet();
  int a, b, k, p[25];
  memset(p, 0, sizeof p);
  for (int j = 0; j < pset.size(); ++j) {
    int x = pset[j];
    Game::Compare(x, g, &a, &b);
    k = a * 5 + b;
    p[k]++;
  }
  return Entropy(p, 25);
}

// static
double SmartPlayer::Entropy(int a[], int n) {
  int s = 0;
  for (int i = 0; i < n; ++i)
    s += a[i];
  double e = 0.0;
  for (int i = 0; i < n; ++i) {
    if (a[i] == 0) continue;
    double p = 1.0 * a[i] / s;
    e -= p * log(p);
//    e -= p * p;
//    e = std::max(e, p);
  }
  return e;
}

// ----------------- GreedyPlayer ------------------

GreedyPlayer::GreedyPlayer() : analyst_(new GameAnalyst) {
}

GreedyPlayer::~GreedyPlayer() {
  delete analyst_;
}

int GreedyPlayer::Think() {
  const vector<int>& pset = analyst_->PSet();
  return guess_ = pset[rand() % pset.size()];
}

void GreedyPlayer::Info(int a, int b) {
  analyst_->Update(guess_, a, b);
}

// ----------------- IdiotPlayer ------------------

IdiotPlayer::IdiotPlayer() {
}

IdiotPlayer::~IdiotPlayer() {
}

int IdiotPlayer::Think() {
  return Game::RandomState();
}

void IdiotPlayer::Info(int a, int b) {
}

}  // namespace master_mind
