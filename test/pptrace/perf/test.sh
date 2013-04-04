#!/bin/sh

if [ -z "$1" ]; then
	echo "usage: $0 [m MATRIX_SIZE] nbit1 nbit2 .. nbitn"
	exit 1
fi


echo "# Arguments: $*"
dprogcreated=0
echo -n  "# It.	Static	Dynamic	PRELOAD	ptrace"
if [ -x "instrumentation/ddyninst" -a -f "./dyninst.conf" ]; then
	./instrument.sh prog dprog
	./instrument.sh mprog mdprog
        dprogcreated=1
	echo -n "	DynInst	DynInst-ptrace"
fi

if [ -x "instrumentation/pin_dummy.so" -a -f "./pin.conf" ]; then
	echo -n "	PIN"
fi

if [ -x "instrumentation/pin_fast.so" -a -f "./pin.conf" ]; then
	echo -n "	PIN-fast"
fi

if [ -x "instrumentation/pin_probed.so" -a -f "./pin.conf" ]; then
	echo -n "	PIN-probed"
fi

mat=0
for i in $*; do 
	if [ "m" = "$i" ]; then
		mat="-1"
	elif [ "$mat" -lt "0" ]; then
		mat="$i"
		echo "# Matrix $i x $i"
	elif [ "$mat" -gt "0" ]; then
		./onetest.sh "$i" "$mat"
	else
		./onetest.sh "$i"
	fi
done

if [ dprogcreated -a -f dprog ]; then
	rm -f dprog mdprog
fi

