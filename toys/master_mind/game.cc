#include "game.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "player.h"

namespace master_mind {

Game::Game(Player* player) :
  player_(player),
  vebose_(false) {
  secret_ = RandomState();
  moves_ = 0;
  wrong_move_ = false;
  won_ = false;
}

void Game::Run() {
  Msg("Game started.\n");
  player_->Prepare();
  while (moves_ < kMaxMoves) {
    int guess = player_->Think();
    int a, b;
    if (!IsStateValid(guess)) {
      wrong_move_ = true;
      Msg("Wrong move.\n");
      break;
    }
    moves_++;
    Compare(secret_, guess, &a, &b);
    Msg("%04d: %dA%dB\n", guess, a, b);
    player_->Info(guess, a, b);
    if (guess == secret_) {
      won_ = true;
      Msg("Won! Moves: %d.\n", moves_);
      break;
    }
  }
  if (!won_ && !wrong_move_)
    Msg("Lost.\n");
  player_->Leave();
}

void Game::Msg(const char* format, ...) {
  if (!vebose_)
    return;

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void Game::SetSecret(int secret) {
  assert(IsStateValid(secret));
  secret_ = secret;
}

void Game::SetVebose(bool vebose) {
  vebose_ = vebose;
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
void Game::AppendAllStates(vector<int>* pset) {
  for (int x = 0; x < 10000; x++)
    if (IsStateValid(x))
      pset->push_back(x);
}

// static.
const int Game::kMaxMoves = 10;

// static.
void Game::Init() {
  srand(time(0));
}

}  // namespace master_mind
