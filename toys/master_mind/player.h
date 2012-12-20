#ifndef MASTER_MIND_PLAYER_H_
#define MASTER_MIND_PLAYER_H_

namespace master_mind {

/* This is the player interface.
 * Typical call sequence for a single game:
 *   Prepare()
 *   Think()
 *   Info()
 *   Think()
 *   Info()
 *   ...
 *   Leave()
 */
class Player {
 public:
  Player();
  virtual ~Player();

  // Called before each game.
  virtual void Prepare() = 0;

  // Called next move.
  virtual int Think() = 0;

  // Called to info the result to the previous move.
  virtual void Info(int guess, int a, int b) = 0;

  // Called after each game.
  virtual void Leave() = 0;
};

class GameAnalyst;

class HumanPlayer : public Player {
 public:
  HumanPlayer();
  virtual ~HumanPlayer();

  virtual void Prepare();
  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Leave();

 private:
  GameAnalyst* analyst_;
};

class GreedyPlayer : public Player {
 public:
  GreedyPlayer();
  virtual ~GreedyPlayer();

  virtual void Prepare();
  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Leave();

 private:
  GameAnalyst* analyst_;
};

class SmartPlayer : public Player {
 public:
  SmartPlayer();
  virtual ~SmartPlayer();

  void Init();

  virtual void Prepare();
  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Leave();

 private:
  struct DecisionTree {
    int guess;
    DecisionTree* child[5 * 5];
  };

  void Build();
  DecisionTree* BuildTree(const GameAnalyst& analyst, int depth);
  void Free();
  void FreeTree(DecisionTree*);

  double DecisionScore(const GameAnalyst& analyst, int g);
  virtual double Score(int a[], int n);

  DecisionTree* root_;
  DecisionTree* node_;
};

class SquarePlayer : public SmartPlayer {
 public:
  SquarePlayer();
  virtual ~SquarePlayer();

 private:
  double Score(int a[], int n);
};

class IdiotPlayer : public Player {
 public:
  IdiotPlayer();
  virtual ~IdiotPlayer();

  virtual void Prepare();
  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Leave();
};

}  // namespace master_mind

#endif  // MASTER_MIND_PLAYER_H_
