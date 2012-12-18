#include "game.h"

#include <stdlib.h>
#include <time.h>

#include "player.h"

namespace master_mind {

Game::Game(Player* player) :
  player_(player) {
  Reset();
}

void Game::Reset() {
  secret_ = RandomState();
  moves_ = 0;
  wrong_move_ = false;
  won_ = false;
}

void Game::Run() {
  while (moves_ < kMaxMoves) {
    int guess = player_->Think();
    int a, b;
    if (!IsStateValid(guess)) {
      wrong_move_ = true;
      break;
    }
    moves_++;
    Compare(secret_, guess, &a, &b);
    player_->Info(a, b);
    if (guess == secret_) {
      won_ = true;
      break;
    }
  }
}

int Game::Moves() const {
  return moves_;
}

bool Game::IsEnded() const {
  return won_ || moves_ >= kMaxMoves || wrong_move_;
}

bool Game::IsWon() const {
  return won_;
}

// static.
void Game::Compare(int secret, int guess, int* a, int* b) {
  int ds[4], dg[4];
  for (int i = 0; i < 4; i++) {
    ds[i] = secret % 10;
    secret /= 10;
    dg[i] = guess % 10;
    guess /= 10;
  }
  *a = *b = 0;
  for (int i = 0; i < 4; i++) {
    if (ds[i] == dg[i]) ++*a;
    for (int j = 0; j < 4; j++) {
      if (i == j) continue;
      if (ds[i] == dg[j]) ++*b;
    }
  }
}

// static.
int Game::RandomState() {
  while (true) {
    int state = rand() % 10000;
    if (IsStateValid(state))
      return state;
  }
}

// static
bool Game::IsStateValid(int state) {
  if (state < 0 || state >= 10000)
    return false;

  int ds[4];
  for (int i = 0; i < 4; i++) {
    ds[i] = state % 10;
    state /= 10;
  }
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < i; j++)
      if (ds[i] == ds[j])
        return false;
  return true;
}

// static.
const int Game::kMaxMoves = 10;

// static.
void Game::Init() {
  srand(time(0));
}

}  // namespace master_mind