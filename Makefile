all:
	g++ -fopenmp -O3 -Wall -Werror -o test src/*.cpp tst/*.cpp
debug:
	g++ -fopenmp -gdwarf-2 -Wall -Werror -o test src/*.cpp tst/*.cpp

clean:
	rm -rf test *.0 *~ *.*~
