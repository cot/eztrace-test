/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

/* This file contains an alternative libexample.so
 * The functions defined in libexample (example_function1 and
 * example_function2) are redefined so that when the application
 * call one of these functions, the hereby version of it is actually
 * invoked.
 */

/* In order to compile this library, you need to add -ldl 
 */

#include "eztrace.h"
#include "example_ev_codes.h"

/* pointers to the actual functions (located in the original
 * libexample.so)
 */
double (*libexample_function1)(double*, int);
double (*libstatic_example_function)(double*, int);
int (*libexample_function2)(int*, int);

/* redefine example_function1.
 * This version of the function only record some events and calls
 * the original example_function1 function
 */
double example_function1(double *array, int array_size)
{
  /* record an event with code FUT_EXAMPLE_FUNCTION1_ENTRY
   * and two values
   */
  EZTRACE_EVENT2(FUT_EXAMPLE_FUNCTION1_ENTRY, array, array_size);

  /* call the actual function (located in the original libexample.so) */
  double ret = libexample_function1(array, array_size);

  /* record another event with code FUT_EXAMPLE_FUNCTION1_ENTRY
   * and two values
   */
  EZTRACE_EVENT2(FUT_EXAMPLE_FUNCTION1_EXIT, array, array_size);
  return ret;
}

double static_example_function(double*array, int array_size)
{
  EZTRACE_EVENT2(FUT_STATIC_EXAMPLE_FUNCTION_ENTRY, array, array_size);

  /* call the actual function (located in the original libexample.so) */
  double ret = libstatic_example_function(array, array_size);

  /* record another event with code FUT_EXAMPLE_FUNCTION1_ENTRY
   * and two values
   */
  EZTRACE_EVENT2(FUT_STATIC_EXAMPLE_FUNCTION_EXIT, array, array_size);
  return ret;
}

/* redefine example_function2.
 * This version of the function only record some events and calls
 * the original example_function2 function
 */
int example_function2(int *array, int array_size)
{
  EZTRACE_EVENT2(FUT_EXAMPLE_FUNCTION2_ENTRY, array, array_size);
  int ret = libexample_function2(array, array_size);
  EZTRACE_EVENT2(FUT_EXAMPLE_FUNCTION2_EXIT, array, array_size);
  return ret;
}

void __example_init (void) __attribute__ ((constructor));
/* Initialize the current library */
void
__example_init (void)
{
  /* store the address of the example_function1 in libexample_function1
   * so that the redefined version of this function can call the
   * original one
   */
  INTERCEPT("example_function1", libexample_function1);
  INTERCEPT("example_function2", libexample_function2);

  INTERCEPT("static_example_function", libstatic_example_function);

  /* start event recording */
#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#endif
}

void __example_conclude (void) __attribute__ ((destructor));
void
__example_conclude (void)
{
  /* stop event recording */
  eztrace_stop ();
}
