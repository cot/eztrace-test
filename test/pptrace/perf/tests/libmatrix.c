/*
 * libmatrix.c
 *
 *  Created on: 28 Sep. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <stdlib.h>
#include "alacon.h"

double *matrix1;
double *matrix2;
int _h;
int _w;

int foo(int h, int w)
{
	int i,j;
	_h = h;
	_w = w;
	matrix1 = (double*)malloc(sizeof(double)*h*w);
	matrix2 = (double*)malloc(sizeof(double)*h*w);
	for(i = 0; i < h; i++) {
		for(j = 0; j < w; j++) {
			matrix1[i*w+j] = (i+1)*(j+1); 
			matrix2[i+j*h] = (i+1)*(j+1); 
		}
	}
	return (h + w);
}

int bar()
{
	int i, j, k;
	double *result = (double*)malloc(sizeof(double)*_h*_h);
	for(i = 0; i < _h; i++) {
		for(j = 0; j < _h; j++) {
			result[i*_h+j] = 0;
			for(k = 0; k < _w; k++) {
				result[i*_h+j] += matrix1[i*_w+k]*matrix2[k*_h+j];
			}
		}
	}
	free(result);
	free(matrix1);
	free(matrix2);
	return 42;
}

