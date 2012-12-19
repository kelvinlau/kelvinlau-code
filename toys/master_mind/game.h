#ifndef MASTER_MIND_GAME_H_
#define MASTER_MIND_GAME_H_

#include <vector>

namespace master_mind {

using std::vector;

class Player;

class Game {
 public:
  Game(Player* player);

  void Run();

  void SetVebose(bool vebose);

  int Moves() const;
  bool IsEnded() const;
  bool IsWon() const;

  static bool IsStateValid(int state);
  static int RandomState();
  static void AppendAllStates(vector<int>* pset);
  static void Compare(int secret, int guess, int* a, int* b);
  static void Init();

  static const int kMaxMoves;

 private:
  void Msg(const char* format, ...);

  int secret_;
  int moves_;
  bool won_;
  bool wrong_move_;
  Player* player_;
  bool vebose_;
};

}  // namespace master_mind

#endif  // MASTER_MIND_GAME_H_
