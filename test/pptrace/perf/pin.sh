#!/bin/sh

. ./timing.sh

if [ -z "$1" -o -z "$2" -o -z "$3" -o -z "$4" ]; then
	echo "usage: $0 pin_module prog iterations matrix_size [time]" >&2
	exit 1
fi

if [ -x ./instrumentation/pin_$1.so -a -f ./pin.conf ]; then
	. ./pin.conf
	if [ -z "$5" ]; then
		$PIN_KIT/pin -t ./instrumentation/pin_$1.so -- tests/$2 $3 $4
	else
		x=`2>&1 time -p $PIN_KIT/pin -t ./instrumentation/pin_$1.so -- tests/$2 $3 $4 | grep user | cut -d " " -f 2`
		echo -n "$x     "
	fi
fi

exit 0