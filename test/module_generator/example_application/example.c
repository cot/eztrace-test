/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "example.h"

#define TIME_DIFF(t1, t2) \
	((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

void compute(unsigned usec)
{
  struct timeval tv1,tv2;
  gettimeofday(&tv1,NULL);
  do {
	gettimeofday(&tv2,NULL);
  } while(TIME_DIFF(tv1, tv2)<usec);
}

int example_do_event(int n) {
  fprintf(stderr, "Doing function #%d\n", n);
  return n+1;
}

/* dummy function #1 */
double example_function1(double *array, int array_size)
{
  compute(10);
  return 0;
}

/* dummy function #2 */
int example_function2(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_fcall(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_push(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_pop(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_event(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_set_var(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_set_var2(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_set_var3(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_set_var4(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_set_var5(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_add_var(int *array, int array_size)
{
  compute(10);
  return 0;
}

int example_sub_var(int *array, int array_size)
{
  compute(10);
  return 0;
}
