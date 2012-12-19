#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "player.h"

namespace master_mind {

void Run(Player* player) {
  Game game(player);
  game.SetVebose(true);
  game.Run();
}

void Benchmark(Player* player, int num_games) {
  printf("NOT IMPLEMENTED.\n");
}

int match(const char* str, const char* pat_) {
  char* pat = strdup(pat_);
  char* save_ptr;
  char* token;
  int ret = 0;
  for (token = strtok_r(pat, "|", &save_ptr);
       token;
       token = strtok_r(NULL, "|", &save_ptr)) {
    ret |= !strcmp(token, str);
  }
  free(pat);
  return ret;
}

Player* GetPlayerByName(const char* name) {
  if (match(name, "smart|s"))
    return new SmartPlayer;
  if (match(name, "idiot|i"))
    return new IdiotPlayer;
  if (match(name, "greedy|g"))
    return new GreedyPlayer;
  if (match(name, "human|h"))
    return new HumanPlayer;
  return NULL;
}

int Main(int argc, char** argv) {
  if (argc == 1) {
    printf("Usage: %s benchmark|b <player> <num-games>\n"
           "       %s run|r <player>\n"
           "\n"
           "<player>:\n"
           "  human|h: Human player;\n"
           "  smart|s: Smart player (default);\n"
           "\n"
           "<num-games>: Number of games to test. (default 100)\n",
           argv[0], argv[0]);
    return 1;
  }

  int op;
  if (match(argv[1], "benchmark|b")) {
    op = 0;
  } else if (match(argv[1], "run|r")) {
    op = 1;
  } else {
    printf("Wrong operator: %s\n", argv[1]);
    return 1;
  }

  const char* player_name = argc > 2 ? argv[2] : "s";
  Player* player = GetPlayerByName(player_name);
  if (player == NULL) {
    printf("No such player: %s\n", player_name);
    return 1;
  }

  Game::Init();

  if (op == 0) {
    int num_games = 100;
    if (argc > 3)
      sscanf(argv[3], "%d", &num_games);
    Benchmark(player, num_games);

  } else if (op == 1) {
    Run(player);
  }

  delete player;
  return 0;
}

}  // namespace master_mind

int main(int argc, char** argv) {
  return master_mind::Main(argc, argv);
}
