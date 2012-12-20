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
  int guess = 0;
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
      sscanf(line, "%d", &guess);
      break;
    }
  }
  return guess;
}

void HumanPlayer::Info(int guess, int a, int b) {
  analyst_->Update(guess, a, b);
}

void HumanPlayer::Reset() {
  delete analyst_;
  analyst_ = new GameAnalyst;
}

// ----------------- SmartPlayer ------------------

SmartPlayer::SmartPlayer() {
  GameAnalyst analyst;
  root_ = BuildTree(analyst, 1);
  node_ = root_;
}

SmartPlayer::~SmartPlayer() {
  FreeTree(root_);
}

// Build a decision tree.
SmartPlayer::DecisionTree* SmartPlayer::BuildTree(const GameAnalyst& analyst,
                                                  int depth) {
  const vector<int>& pset = analyst.PSet();
  if (pset.empty()) return NULL;

  DecisionTree* node = new DecisionTree;

  if (depth == 1) {
    node->guess = 1234;  // Fixed number at first guess.
  } else {
    double emax = -1e100;
    for (int i = 0; i < pset.size(); ++i) {
      int g = pset[i];
      double e = DecisionEntropy(analyst, g);
      if (emax < e) {
        emax = e;
        node->guess = g;
      }
    }
  }
  for (int a = 0; a <= 4; a++) {
    for (int b = 0; a + b <= 4; b++) {
      int k = a * 5 + b;
      if (a == 4 && b == 0) {
        node->child[k] = NULL;
        continue;
      }
      GameAnalyst analyst1 = analyst;  // Make a copy.
      analyst1.Update(node->guess, a, b);
      node->child[k] = BuildTree(analyst1, depth + 1);
    }
  }
  return node;
}

// Free a decision tree.
void SmartPlayer::FreeTree(DecisionTree* node) {
  if (!node)
    return;
  for (int a = 0; a <= 4; a++) {
    for (int b = 0; a + b <= 4; b++) {
      int k = a * 5 + b;
      FreeTree(node->child[k]);
    }
  }
  delete node;
}

double SmartPlayer::DecisionEntropy(const GameAnalyst& analyst, int g) {
  const vector<int>& pset = analyst.PSet();
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
//    e = std::min(e, -p);
  }
  return e;
}

int SmartPlayer::Think() {
  return node_->guess;
}

void SmartPlayer::Info(int guess, int a, int b) {
  int k = a * 5 + b;
  node_ = node_->child[k];
}

void SmartPlayer::Reset() {
  node_ = root_;
}

// ----------------- GreedyPlayer ------------------

GreedyPlayer::GreedyPlayer() : analyst_(new GameAnalyst) {
}

GreedyPlayer::~GreedyPlayer() {
  delete analyst_;
}

int GreedyPlayer::Think() {
  const vector<int>& pset = analyst_->PSet();
  return pset[rand() % pset.size()];
}

void GreedyPlayer::Info(int guess, int a, int b) {
  analyst_->Update(guess, a, b);
}

void GreedyPlayer::Reset() {
  delete analyst_;
  analyst_ = new GameAnalyst;
}

// ----------------- IdiotPlayer ------------------

IdiotPlayer::IdiotPlayer() {
}

IdiotPlayer::~IdiotPlayer() {
}

int IdiotPlayer::Think() {
  return Game::RandomState();
}

void IdiotPlayer::Info(int guess, int a, int b) {
}

void IdiotPlayer::Reset() {
}

}  // namespace master_mind
