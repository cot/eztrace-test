# Copyright © CNRS, INRIA, Université Bordeaux 1
# See COPYING in top-level directory.

BINARIES = vector

all: $(BINARIES)

vector: vector.cxx
	$(CXX) -o vector vector.cxx -O3 $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f  *.o $(BINARIES)
