CC=g++
FLAGS=-std=c++11 -g -O0
ADDRESS_SANITIZER=-fsanitize=address
CFLAGS=-DVERBOSE=0

all: bin/histogram bin/conflicting-addresses bin/detect-col-bits bin/verify-dram-mapping bin/hammer bin/time-act bin/press | bin
clean:
	rm -rf bin/

bin/histogram: src/histogram/histogram.cc src/shared.cc src/shared.hh src/params.hh src/util.hh | bin
	$(CC) $(CFLAGS) $(FLAGS) -o $@ src/histogram/histogram.cc src/util.hh src/shared.cc

bin/conflicting-addresses: src/reverse/conflicting-addresses.cc src/shared.cc src/shared.hh src/params.hh src/util.hh | bin
	$(CC) $(CFLAGS) $(FLAGS) -o $@ src/reverse/conflicting-addresses.cc src/util.hh src/shared.cc

bin/verify-dram-mapping: src/reverse/verify-dram-mapping.cc src/shared.cc src/shared.hh src/params.hh src/util.hh | bin
	$(CC) $(CFLAGS) $(FLAGS) -o $@ src/reverse/verify-dram-mapping.cc src/util.hh src/shared.cc

bin/detect-col-bits: src/reverse/detect-col-bits.cc src/shared.cc src/shared.hh src/params.hh src/util.hh | bin
	$(CC) $(CFLAGS) $(FLAGS) -o $@ src/reverse/detect-col-bits.cc src/util.hh src/shared.cc

bin/hammer: src/hammer/hammer.cc src/shared.cc src/shared.hh src/params.hh src/util.hh | bin
	$(CC) $(CFLAGS) $(FLAGS) $(ADDRESS_SANITIZER) -o $@ src/hammer/hammer.cc src/util.hh src/shared.cc

bin/time-act: src/hammer/time-act.cc src/shared.cc src/shared.hh src/params.hh src/util.hh | bin
	$(CC) $(CFLAGS) $(FLAGS) -o $@ src/hammer/time-act.cc src/util.hh src/shared.cc

bin/press: src/press/press.cc src/shared.cc src/shared.hh src/params.hh src/util.hh | bin
	$(CC) $(CFLAGS) $(FLAGS) $(ADDRESS_SANITIZER) -o $@ src/press/press.cc src/util.hh src/shared.cc

bin:
	mkdir bin
