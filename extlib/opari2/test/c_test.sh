#!/bin/sh
# This file is part of the Score-P software (http://www.score-p.org)
# 
# Copyright (c) 2009-2011,
#    *    RWTH Aachen University, Germany
#    *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
#    *    Technische Universitaet Dresden, Germany
#    *    University of Oregon, Eugene, USA
#    *    Forschungszentrum Juelich GmbH, Germany
#    *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
#    *    Technische Universitaet Muenchen, Germany
#  
# See the COPYING file in the package base directory for details.

opari_dir=`pwd`
test_dir="../test/tmp"
mkdir -p ${test_dir}
test_data_dir=/home/rue/EZTRACE/eztrace/unused-git/extlib/opari2/test/data
sed=/bin/sed
awk=gawk

echo $test_data_dir
echo $test_dir
cp $test_data_dir/test?.c $test_dir/

if [ ! -f $test_dir/replacePaths_c.awk ]; then
    cp $test_data_dir/../replacePaths_c.awk $test_dir/
fi
cd $test_dir

for file in test?.c
do 
  echo "        $file ..."
  base=`basename $file .c`
  $opari_dir/opari2 --untied=keep,no-warn $file || exit

# Replace the full path in the line numbering 
#  in the source files
  `$awk -f replacePaths_c.awk $base.mod.c > $base.mod.c.tmp`
  `mv $base.mod.c.tmp $base.mod.c`

# Replace the full paths and unify timestamp based region identifiers
#  in the include files
  `$awk -f replacePaths_c.awk $base.c.opari.inc > $base.c.opari.inc.tmp`
  `mv $base.c.opari.inc.tmp $base.c.opari.inc`
  if diff -u $test_data_dir/$base.c.out $base.mod.c > /dev/null
  then
    true
  else
    echo "-------- ERROR: unexpected change in transformed program --------"
    diff -u $test_data_dir/$base.c.out $base.mod.c
    error="true"
    continue
  fi
  if diff -u $test_data_dir/$base.c.opari.inc.out $base.c.opari.inc > /dev/null
  then
    true
  else
    echo "-------- ERROR: unexpected change in opari include file --------"
    diff -u $test_data_dir/$base.c.opari.inc.out $base.c.opari.inc
    error="true"
    continue 
  fi
done

cd $opari_dir

if [ "$error" = "true" ]
then
	exit -1
fi
