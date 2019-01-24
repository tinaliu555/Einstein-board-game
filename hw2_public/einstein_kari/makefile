OP = -std=c++11 -O2 -Wall -Wno-unused-result
all:
	g++ $(OP) einstein.cc -c
	g++ $(OP) game.cc agent.cc getch.cc einstein.o -o game
	g++ $(OP) greedy.cc einstein.o -o greedy
	g++ $(OP) random.cc einstein.o -o random
	rm -f *.o
clean:
	rm -f game greedy random
