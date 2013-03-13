/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */
#include <stdio.h>
#include "example.h"

/* dummy function #1 */
double example_static_function(double *array, int array_size)
{
  int i;
  double sum = 0;
  for(i=0;i<array_size;i++) {
    array[i] = array[i] * (i+1);
    sum += array[i];
  }
  return sum;
}
