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

int main(int argc, char**argv)
{
	double *double_array = malloc(sizeof(double)*SIZE);
	int *int_array = malloc(sizeof(int)*SIZE);
	int i;

	/* initialize the array with something */
	for(i=0; i<SIZE; i++) {
		double_array[i] = (i*i)/31;
	}

	double sum_double = example_function1(double_array, SIZE);

	/* initialize the array with something */
	for(i=0; i<SIZE; i++) {
		double_array[i] = (i*i)/31;
	}

	double static_sum_double = example_static_function(double_array, SIZE);

	for(i=0; i<SIZE; i++) {
		int_array[i] = i%17;
	}
	int sum_int = example_function2(int_array, SIZE);

	printf("sum double = %lf\n", sum_double);
	printf("static sum double = %lf\n", static_sum_double);
	printf("sum int = %d\n", sum_int);
	return 0;
}
