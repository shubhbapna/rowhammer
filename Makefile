CC=g++

all: bin/histogram
clean:
	rm -f bin/histogram

bin/histogram: bin src/histogram.cc src/shared.cc src/shared.hh src/params.hh src/util.hh
	$(CC) -std=c++11 -g -o $@ src/histogram.cc src/util.hh src/shared.cc

bin:
	mkdir bin
