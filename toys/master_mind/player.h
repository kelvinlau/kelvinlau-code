#ifndef MASTER_MIND_PLAYER_H_
#define MASTER_MIND_PLAYER_H_

#include <vector>

using std::vector;

namespace master_mind {

class Player {
 public:
  Player();
  virtual ~Player();

  virtual int Think() = 0;
  virtual void Info(int a, int b) = 0;
};

class HumanPlayer : public Player {
 public:
  HumanPlayer();
  virtual ~HumanPlayer();

  virtual int Think();
  virtual void Info(int a, int b);

 private:
  int guess_;
  vector<int> pset_;
};

}  // namespace master_mind

#endif  // MASTER_MIND_PLAYER_H_
