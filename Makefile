CC=g++

all: bin/gather
clean:
	rm -f bin/gather

bin/gather: bin src/gather.cc src/shared.cc src/shared.hh src/params.hh src/util.hh
	$(CC) -std=c++11 -o $@ src/gather.cc src/util.hh src/shared.cc

bin:
	mkdir bin
