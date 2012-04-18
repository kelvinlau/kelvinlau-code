all: main

CC = gcc
CFLAGS = -lm -g

boosting.o: boosting.c boosting.h hash_table.h decision_tree.h
decision_tree.o: decision_tree.c decision_tree.h hash_table.h
hash_table.o: hash_table.c hash_table.h
main.o: main.c hash_table.h boosting.h

main: main.o hash_table.o decision_tree.o boosting.o
	$(CC) $(CFLAGS) -o main main.o hash_table.o decision_tree.o boosting.o

run: main
	./main ../../data/train.dat ../../data/test.dat

clean:
	rm -f *.o main
