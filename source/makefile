







OP = -std=c++11 -O2 -Wall -Wno-unused-result -fopenmp
all:
	g++ $(OP) einstein.cc -c
	g++ $(OP) r06922041.cc einstein.o -o r06922041
	rm -f *.o
clean:
	rm -f r06922041
