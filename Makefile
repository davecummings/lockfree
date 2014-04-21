defule:
	g++ -fopenmp -g -O3 -o test src/*.cpp tst/*.cpp
clean:
	rm -rf test *.0 *~ *.*~
