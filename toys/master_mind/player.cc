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

HumanPlayer::HumanPlayer() {
}

HumanPlayer::~HumanPlayer() {
  delete analyst_;
}

void HumanPlayer::Prepare() {
  analyst_ = new GameAnalyst;
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

void HumanPlayer::Leave() {
  delete analyst_;
}

// ----------------- SmartPlayer ------------------

SmartPlayer::SmartPlayer() {
  root_ = NULL;
}

SmartPlayer::~SmartPlayer() {
  FreeTree(root_);
}

void SmartPlayer::Init() {
  GameAnalyst analyst;
  root_ = BuildTree(analyst, 1);
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
    double smax = -1e100;
    for (int i = 0; i < pset.size(); ++i) {
      int g = pset[i];
      double s = DecisionScore(analyst, g);
      if (smax < s) {
        smax = s;
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

double SmartPlayer::DecisionScore(const GameAnalyst& analyst, int g) {
  const vector<int>& pset = analyst.PSet();
  int a, b, k, p[25];
  memset(p, 0, sizeof p);
  for (int j = 0; j < pset.size(); ++j) {
    int x = pset[j];
    Game::Compare(x, g, &a, &b);
    k = a * 5 + b;
    p[k]++;
  }
  return Score(p, 25);
}

void SmartPlayer::Prepare() {
  node_ = root_;
}

int SmartPlayer::Think() {
  return node_->guess;
}

void SmartPlayer::Info(int guess, int a, int b) {
  int k = a * 5 + b;
  node_ = node_->child[k];
}

void SmartPlayer::Leave() {
}

// ----------------- SquarePlayer ------------------

SquarePlayer::SquarePlayer() {
}

SquarePlayer::~SquarePlayer() {
}

double SquarePlayer::Score(int a[], int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i)
    sum += a[i];
  double s = 0.0;
  for (int i = 0; i < n; ++i) {
    if (a[i] == 0) continue;
    double p = 1.0 * a[i] / sum;
    s -= p * p;
  }
  return s;
}

// ----------------- EntropyPlayer ------------------

EntropyPlayer::EntropyPlayer() {
}

EntropyPlayer::~EntropyPlayer() {
}

double EntropyPlayer::Score(int a[], int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i)
    sum += a[i];
  double s = 0.0;
  for (int i = 0; i < n; ++i) {
    if (a[i] == 0) continue;
    double p = 1.0 * a[i] / sum;
    s -= p * log(p);
  }
  return s;
}

// ----------------- MinMaxPlayer ------------------

MinMaxPlayer::MinMaxPlayer() {
}

MinMaxPlayer::~MinMaxPlayer() {
}

double MinMaxPlayer::Score(int a[], int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i)
    sum += a[i];
  double s = 0.0;
  for (int i = 0; i < n; ++i) {
    if (a[i] == 0) continue;
    double p = 1.0 * a[i] / sum;
    s = std::min(s, -p);
  }
  return s;
}

// ----------------- GreedyPlayer ------------------

GreedyPlayer::GreedyPlayer() {
}

GreedyPlayer::~GreedyPlayer() {
}

// Return a random score so that it will guess random element in pset.
double GreedyPlayer::Score(int a[], int n) {
  return rand() * 1.0;
}

// ----------------- IdiotPlayer ------------------

IdiotPlayer::IdiotPlayer() {
}

IdiotPlayer::~IdiotPlayer() {
}

void IdiotPlayer::Prepare() {
}

int IdiotPlayer::Think() {
  return Game::RandomState();
}

void IdiotPlayer::Info(int guess, int a, int b) {
}

void IdiotPlayer::Leave() {
}

}  // namespace master_mind
