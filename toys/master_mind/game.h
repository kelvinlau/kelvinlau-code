#ifndef MASTER_MIND_GAME_H_
#define MASTER_MIND_GAME_H_

namespace master_mind {

class Player;

class Game {
 public:
  Game(Player* player);

  void Reset();
  void Run();

  int Moves() const;
  bool IsEnded() const;
  bool IsWon() const;

  static bool IsStateValid(int state);
  static void Compare(int secret, int guess, int* a, int* b);
  static void Init();

 private:
  static int RandomState();

  static const int kMaxMoves;

  int secret_;
  int moves_;
  bool won_;
  bool wrong_move_;
  Player* player_;
};

}  // namespace master_mind

#endif  // MASTER_MIND_GAME_H_
