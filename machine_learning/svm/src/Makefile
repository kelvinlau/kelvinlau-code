all: main

CC = gcc
CFLAGS = -lm -g -Wall

main: main.o sparse_list.o svm.o
	$(CC) $(CFLAGS) -o main main.o sparse_list.o svm.o

run: main
	./main ../../data/train.dat ../../data/test.dat

clean:
	rm -f main *.o

main.o: main.c svm.h
sparse_list.o: sparse_list.c sparse_list.h
svm.o: svm.c svm.h sparse_list.h
