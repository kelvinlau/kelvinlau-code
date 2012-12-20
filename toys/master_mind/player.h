#ifndef MASTER_MIND_PLAYER_H_
#define MASTER_MIND_PLAYER_H_

namespace master_mind {

class Player {
 public:
  Player();
  virtual ~Player();

  virtual int Think() = 0;
  virtual void Info(int guess, int a, int b) = 0;
  virtual void Reset() = 0;
};

class GameAnalyst;

class HumanPlayer : public Player {
 public:
  HumanPlayer();
  virtual ~HumanPlayer();

  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Reset();

 private:
  GameAnalyst* analyst_;
};

class GreedyPlayer : public Player {
 public:
  GreedyPlayer();
  virtual ~GreedyPlayer();

  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Reset();

 private:
  GameAnalyst* analyst_;
};

class SmartPlayer : public Player {
 public:
  SmartPlayer();
  virtual ~SmartPlayer();

  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Reset();

 private:
  struct DecisionTree {
    int guess;
    DecisionTree* child[5 * 5];
  };

  void Build();
  DecisionTree* BuildTree(const GameAnalyst& analyst, int depth);
  void Free();
  void FreeTree(DecisionTree*);

  double DecisionEntropy(const GameAnalyst& analyst, int g);
  double Entropy(int a[], int n);

  DecisionTree* root_;
  DecisionTree* node_;
};

class IdiotPlayer : public Player {
 public:
  IdiotPlayer();
  virtual ~IdiotPlayer();

  virtual int Think();
  virtual void Info(int guess, int a, int b);
  virtual void Reset();
};

}  // namespace master_mind

#endif  // MASTER_MIND_PLAYER_H_
