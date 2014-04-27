defule:
	g++ -fopenmp -g -O3 -o test src/coarse_grained_list.cpp  tst/*.cpp
clean:
	rm -rf test *.0 *~ *.*~
