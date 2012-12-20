#ifndef MASTER_MIND_PLAYER_H_
#define MASTER_MIND_PLAYER_H_

namespace master_mind {

class Player {
 public:
  Player();
  virtual ~Player();

  virtual int Think() = 0;
  virtual void Info(int a, int b) = 0;  // TODO: add guess.
};

class GameAnalyst;

class HumanPlayer : public Player {
 public:
  HumanPlayer();
  virtual ~HumanPlayer();

  virtual int Think();
  virtual void Info(int a, int b);

 private:
  int guess_;
  GameAnalyst* analyst_;
};

class GreedyPlayer : public Player {
 public:
  GreedyPlayer();
  virtual ~GreedyPlayer();

  virtual int Think();
  virtual void Info(int a, int b);

 private:
  int guess_;
  GameAnalyst* analyst_;
};

class SmartPlayer : public Player {
 public:
  SmartPlayer();
  virtual ~SmartPlayer();

  virtual int Think();
  virtual void Info(int a, int b);

 private:
  struct DecisionTree {
    int guess;
    DecisionTree* child[5 * 5];
  };

  static DecisionTree* root;

  static void Build();
  static DecisionTree* BuildTree(const GameAnalyst& analyst, int depth);
  static void Free();
  static void FreeTree(DecisionTree*);

  static double DecisionEntropy(const GameAnalyst& analyst, int g);
  static double Entropy(int a[], int n);

  DecisionTree* node_;
};

class IdiotPlayer : public Player {
 public:
  IdiotPlayer();
  virtual ~IdiotPlayer();

  virtual int Think();
  virtual void Info(int a, int b);

 private:
};

}  // namespace master_mind

#endif  // MASTER_MIND_PLAYER_H_
