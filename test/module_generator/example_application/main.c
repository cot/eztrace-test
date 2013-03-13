/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

/* simple program that calls the libexample library */

#include <stdlib.h>
#include <stdio.h>
#include "example.h"

#define SIZE (1024*1024)

void appli_function1(double* array, int size) {
  fprintf(stderr, "appli function1 (%p, %d)\n", array, size);
  compute(10);
}

int main(int argc, char**argv)
{
  double *double_array = NULL;
  int *int_array = NULL;
  int func_num=0;

  func_num = example_do_event(func_num);
  example_function1(double_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_function2(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_fcall(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_push(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_pop(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_event(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_set_var(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_add_var(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_sub_var(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_set_var2(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_set_var3(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_set_var4(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  example_set_var5(int_array, SIZE);
  compute(20);

  func_num = example_do_event(func_num);
  appli_function1(double_array, SIZE);
  compute(20);

  return 0;
}
