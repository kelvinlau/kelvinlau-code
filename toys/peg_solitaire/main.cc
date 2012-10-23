#include "peg_solitaire.h"

int main(int argc, char **argv) {
  PegSolitaire peg_solitaire;
  if (argv[1][0] == 's')
    peg_solitaire.Search(10);
  else
    peg_solitaire.Bfs();
  return 0;
}
