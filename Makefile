CC=g++

all: bin/histogram histogram
clean:
	rm -f bin/histogram

histogram: bin/histogram src/histogram.py data
	./bin/histogram > data/histogram.out
	python3 src/histogram.py
	cat data/histogram.out

bin/histogram: bin src/histogram.cc src/shared.cc src/shared.hh src/params.hh src/util.hh
	$(CC) -std=c++11 -g -o $@ src/histogram.cc src/util.hh src/shared.cc

bin:
	mkdir bin

data:
	mkdir data
