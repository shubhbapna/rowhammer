CC=g++

all: bin/histogram histogram bin/reverse
clean:
	rm -f bin/histogram

histogram: bin/histogram src/histogram.py data
	./bin/histogram > data/histogram.out
	python3 src/histogram.py
	cat data/histogram.out

bin/histogram: bin src/histogram.cc src/shared.cc src/shared.hh src/params.hh src/util.hh
	$(CC) -std=c++11 -g -O0 -o $@ src/histogram.cc src/util.hh src/shared.cc

bin/reverse: bin src/reverse.cc src/shared.cc src/shared.hh src/params.hh src/util.hh
	$(CC) -std=c++11 -g -O0 -o $@ src/reverse.cc src/util.hh src/shared.cc

bin/reverse-rows: bin src/reverse-rows.cc src/shared.cc src/shared.hh src/params.hh src/util.hh
	$(CC) -std=c++11 -g -O0 -o $@ src/reverse-rows.cc src/util.hh src/shared.cc

bin:
	mkdir bin

data:
	mkdir data
