#include "player.h"

#include <stdio.h>
#include <string.h>

#include "game.h"

namespace master_mind {

// ----------------- Player ------------------

Player::Player() {}

Player::~Player() {}

// ----------------- HumanPlayer ------------------

HumanPlayer::HumanPlayer() {
  for (int x = 0; x < 10000; x++)
    if (Game::IsStateValid(x))
      pset_.push_back(x);
}

HumanPlayer::~HumanPlayer() {}

int HumanPlayer::Think() {
  char line[1 << 10];
  guess_ = 0;
  while (true) {
    printf(">> ");
    if (!fgets(line, sizeof line, stdin))
      break;
    line[strlen(line) - 1] = '\0';  // Strip newline.
    if (strcmp(line, "ls") == 0) {
      for (int i = 0; i < pset_.size(); ++i)
        printf("%04d    ", pset_[i]);
      puts("");
    } else {
      sscanf(line, "%d", &guess_);
      break;
    }
  }
  return guess_;
}

void HumanPlayer::Info(int a, int b) {
  printf("%d: %dA%dB\n", guess_, a, b);

  vector<int> pset;
  for (int i = 0; i < pset_.size(); ++i) {
    int a1, b1;
    Game::Compare(pset_[i], guess_, &a1, &b1);
    if (a1 == a && b1 == b) {
      pset.push_back(pset_[i]);
    }
  }
  pset_.swap(pset);
}

}  // namespace master_mind
