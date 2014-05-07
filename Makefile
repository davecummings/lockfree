CC=g++
CFLAGS=-std=c++11 -Wall -Werror -o test src/*.cpp tst/*.cpp
MAC_SOURCES=src/*.cpp tst/smc.cpp tst/monitor.cpp tst/test_device.cpp
LINUX_SOURCES=src/*.cpp tst/monitor.cpp tst/test.cpp

all:
	$(CC) $(CFLAGS) $(LINUX_SOURCES) -fopenmp -O3

debug:
	$(CC) $(CFLAGS) $(LINUX_SOURCES) -fopenmp -gdwarf-2

mac:
	$(CC) $(CFLAGS) -O3 $(MAC_SOURCES) -framework CoreFoundation -framework IOKit

clean:
	rm -rf test *.0 *~ *.*~
