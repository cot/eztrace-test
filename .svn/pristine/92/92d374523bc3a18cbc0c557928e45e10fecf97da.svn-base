#!/bin/sh

if [ ! -f "./dyninst.conf" ]; then
	exit 1
fi

if [ -z "$1" -o -z "$2" -o -z "$3" -o -z "$4" ]; then
	echo "Usage: $0 progname destination arg1 arg2" >&2
	exit 1
fi

. ./dyninst.conf
. ./timing.sh
LD_PATH=$DYNINST_LIB:

./instrument.sh $1 $2
dprogcreated=$$
if [ "$dprogcreated" -ge 0 ]; then
        timing "" ./$2 $3 $4
        if [ "$dprogcreated" -gt 0 ]; then
                rm $2
        fi
fi      

exit 0

