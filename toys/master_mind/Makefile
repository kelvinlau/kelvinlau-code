CXX = g++

all: main

game.o: game.cc game.h player.h
main.o: main.cc game.h player.h
player.o: player.cc player.h

main: main.o game.o player.o
	$(CXX) $^ -o $@

clean:
	rm -rf main *.o
