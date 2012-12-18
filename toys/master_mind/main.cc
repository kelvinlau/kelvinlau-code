#include <stdio.h>

#include "game.h"
#include "player.h"

using master_mind::Game;
using master_mind::Player;
using master_mind::HumanPlayer;

int main() {
  Game::Init();

  HumanPlayer player;
  Game game(&player);
  printf("Game started.\n");
  game.Run();
  if (game.IsEnded()) {
    if (game.IsWon()) {
      printf("Won! Moves: %d.\n", game.Moves());
    } else {
      printf("Lost.\n");
    }
  } else {
    printf("Game is not ended.\n");
  }
  return 0;
}
