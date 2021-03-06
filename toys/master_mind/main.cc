#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "player.h"

namespace master_mind {

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
  if (match(name, "entropy|e")) {
    EntropyPlayer* player = new EntropyPlayer;
    player->Init();
    return player;
  }
  if (match(name, "square|sq")) {
    SquarePlayer* player = new SquarePlayer;
    player->Init();
    return player;
  }
  if (match(name, "minmax|mm")) {
    MinMaxPlayer* player = new MinMaxPlayer;
    player->Init();
    return player;
  }
  if (match(name, "greedy|g")) {
    GreedyPlayer* player = new GreedyPlayer;
    player->Init();
    return player;
  }
  if (match(name, "human|h"))
    return new HumanPlayer;
  if (match(name, "idiot|i"))
    return new IdiotPlayer;
  return NULL;
}

void Run(Player* player, int num_games) {
  int won = 0, sum = 0, max = 0;
  for (int i = 0; i < num_games; ++i) {
    Game game(player);
    game.SetVebose(num_games == 1);
    game.Run();
    assert(game.IsEnded());
    if (game.IsWon()) {
      won++;
      sum += game.Moves();
      max = std::max(max, game.Moves());
    }
  }
  if (num_games > 1)
    printf("Won: %d/%d, Averge: %lf, Max: %d.\n",
           won, num_games, 1.0 * sum / won, max);
}

void Benchmark(Player* player) {
  int won = 0, sum = 0, max = 0;
  vector<int> all;
  Game::AppendAllStates(&all);
  for (int i = 0; i < all.size(); ++i) {
    Game game(player);
    game.SetSecret(all[i]);
    game.SetVebose(false);
    game.Run();
    assert(game.IsEnded());
    if (game.IsWon()) {
      won++;
      sum += game.Moves();
      max = std::max(max, game.Moves());
    }
  }
  printf("Won: %d/%zd, Averge: %lf, Max: %d.\n",
         won, all.size(), 1.0 * sum / won, max);
}

int Main(int argc, char** argv) {
  if (argc == 1) {
    printf("Usage: %s benchmark|bm <player>\n"
           "       %s run <player> <num-games>\n"
           "\n"
           "<player>:\n"
           "  entropy|e: Smart player using entropy scoring function;\n"
           "  square|sq: Smart player using square scoring function;\n"
           "  minmax|mm: Smart player using min-max scoring function;\n"
           "  greedy|g: Smart player using random scoring function;\n"
           "  human|h: Human player;\n"
           "  idiot|i: Idiot player;\n"
           "\n"
           "<num-games>: Number of games to run. (default 1)\n",
           argv[0], argv[0]);
    return 1;
  }

  int op;
  if (match(argv[1], "benchmark|bm")) {
    op = 0;
  } else if (match(argv[1], "run")) {
    op = 1;
  } else {
    printf("Wrong operator: %s\n", argv[1]);
    return 1;
  }

  const char* player_name = argc > 2 ? argv[2] : "";
  Player* player = GetPlayerByName(player_name);
  if (player == NULL) {
    printf("No such player: %s\n", player_name);
    return 1;
  }

  Game::Init();

  if (op == 0) {
    Benchmark(player);

  } else if (op == 1) {
    int num_games = 1;
    if (argc > 3)
      sscanf(argv[3], "%d", &num_games);
    Run(player, num_games);
  }

  delete player;

  return 0;
}

}  // namespace master_mind

int main(int argc, char** argv) {
  return master_mind::Main(argc, argv);
}
