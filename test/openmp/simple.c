/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

# include <stdlib.h>
# include <stdio.h>
# include <time.h>
# include <omp.h>
#include <stdarg.h>


// Debugging part, print out only if debugging level of the system is verbose or more
int _debug = -77;

void debug(char *fmt, ...)
{
	if(_debug == -77) {
		char *buf = getenv("EZTRACE_DEBUG");
		if(buf == NULL) _debug = 0;
		else _debug = atoi(buf);
	}
	if(_debug >= 0) { // debug verbose mode
		va_list va;
		va_start(va, fmt);
		vfprintf(stdout, fmt, va);
		va_end(va);
	}
}
// end of debugging part

#define SIZE (1024*1024*8)

int main(void)
{
	int i;
	int *A = malloc(sizeof(int)*SIZE);
	int *B = malloc(sizeof(int)*SIZE);
	int *C = malloc(sizeof(int)*SIZE);

	for(i=0;i<SIZE; i++)
	{
		A[i]=i*17%7;
		B[i]=i*19%7;
		C[i]=0;
	}


//compute for real!
	int j;
	for(j=0;j<10; j++) {
		debug("loop %d\n", j);
		debug("\trunning parallel for schedule(static)\n");
#pragma omp parallel for schedule(static)
 		for(i=0;i<SIZE; i++)
 		{
 			C[i]=A[i]+B[i];
 		}

		debug("\trunning parallel for schedule(runtime)\n");
#pragma omp parallel for schedule(runtime)
 		for(i=0;i<SIZE; i++)
 		{
 			C[i]=A[i]+B[i];
 		}

		debug("\trunning parallel for schedule(dynamic)\n");
#pragma omp parallel for schedule(dynamic)
		for(i=0;i<SIZE; i++)
		{
			C[i]=A[i]+B[i];
		}

		debug("\trunning parallel for schedule(guided)\n");
#pragma omp parallel for schedule(guided)
 		for(i=0;i<SIZE; i++)
 		{
 			C[i]=A[i]+B[i];
 		}
	}
	return 0;
}
