#!/bin/sh

if [ -z "$1" ]; then
	echo "usage: $0 iterations [matrix_size]" >&2
	exit 1
fi

. ./timing.sh
if [ -z "$2" ]; then
	PROG=prog
	DYNPROG=dynprog
	DPROG=dprog
	LIBALACON=libalacon.so
	MSIZE=0
else
	PROG=mprog
	DYNPROG=dynprog
	DPROG=mdprog
	LIBALACON=libmatrix.so
	MSIZE=$2
fi

cp tests/$LIBALACON libalacon.so

echo -n "$1	"
# static
timing "" tests/$PROG $1 $MSIZE

# dynamic
timing "" tests/$DYNPROG $1 $MSIZE

# preload
timing ./tests/libpreload.so tests/$DYNPROG $1 $MSIZE

# pptrace
timing "" ./instrumentation/pptrace ./tests/libpptrace.so tests/$PROG $1 $MSIZE

rm -f libalacon.so

# dyninst
./dyninst.sh $PROG $DPROG $1 $MSIZE

# dyninst 2
if [ -x ./instrumentation/dyninst -a -f ./dyninst.conf ]; then 
	. ./dyninst.conf
	LD_PATH=$DYNINST_LIB:
	
	timing2 "$DYNINST_LIB/libdyninstAPI_RT.so" ./instrumentation/dyninst tests/$PROG $1 $MSIZE
fi

# pin
./pin.sh dummy $PROG $1 $MSIZE 1

# pin fast
./pin.sh fast $PROG $1 $MSIZE 1

# pin_probed
./pin.sh probed $PROG $1 $MSIZE 1

echo

