CXXFLAGS = -Wall -g

Driver2.out: Sally.h Sally.cpp driver2.cpp
		g++ $(CXXFLAGS) Sally.cpp Sally.h driver2.cpp -o Driver2.out

make clean:
		rm -rf *.o
		rm -rf *~
		rm -rf *.out

make run:
		make;
		./Driver.out