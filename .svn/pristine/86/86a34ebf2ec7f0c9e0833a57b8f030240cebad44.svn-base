#!/bin/sh

timing2()
{
        if [ -z "$1" ]; then
                x=`LD_LIBRARY_PATH=$LD_PATH.:./tests 2>&1 time -p $2 $3 $4 $5 $6 $7 | grep user | cut -d " " -f 2`
        else
                x=`DYNINSTAPI_RT_LIB=$1 LD_LIBRARY_PATH=$LD_PATH.:./tests 2>&1 time -p $2 $3 $4 $5 $6 $7 | grep user | cut -d " " -f 2`
        fi
        echo -n "$x     "
}


timing()
{
        if [ -z "$1" ]; then
                x=`LD_LIBRARY_PATH=$LD_PATH.:./tests 2>&1 time -p $2 $3 $4 $5 $6 $7 | grep user | cut -d " " -f 2`
        else
                x=`LD_PRELOAD=$1 LD_LIBRARY_PATH=$LD_PATH.:./tests 2>&1 time -p $2 $3 $4 $5 $6 $7 | grep user | cut -d " " -f 2`
        fi
        echo -n "$x     "
}