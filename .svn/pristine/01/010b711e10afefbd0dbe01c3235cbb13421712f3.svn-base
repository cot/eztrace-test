#!/bin/sh

if [ -z "$1" -o -z "$2" ]; then
	echo "Usage: $0 progname destination" >&2
	exit -1
fi

. ./dyninst.conf
dyninst=./instrumentation/ddyninst
LIB_PATH=/usr/local/lib:$DYNINST_LIB

if [ ! -x "$dyninst" ]; then
	exit -2
else
	if [ -f "$2" ]; then
		exit 0
	else
		LD_LIBRARY_PATH=$LIB_PATH DYNINSTAPI_RT_LIB=$DYNINST_LIB/libdyninstAPI_RT.so $dyninst tests/$1 $2 >/dev/null
		exit 1
	fi
fi

