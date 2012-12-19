#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
}

SmartPlayer::~SmartPlayer() {
  delete analyst_;
}

int SmartPlayer::Think() {
  const vector<int>& pset = analyst_->PSet();
  return guess_ = pset[0];
}

void SmartPlayer::Info(int a, int b) {
  analyst_->Update(guess_, a, b);
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
