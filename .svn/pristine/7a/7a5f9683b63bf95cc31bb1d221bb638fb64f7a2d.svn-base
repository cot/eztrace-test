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

	omp_lock_t lock;
	omp_init_lock(&lock);

	int somme = 0;
//compute for real!
	int j;
	for(j=0;j<1; j++) {
		debug("loop %d\n", j);
		debug("\trunning parallel for schedule(static)\n");
#pragma omp parallel for
 		for(i=0;i<SIZE; i++)
 		{
		  C[i]=A[i]+B[i];
			if(i%1000 == 0) {
			  omp_set_lock(&lock);
			  somme+= C[i];
			  omp_unset_lock(&lock);
			}
 		}

	}

	printf("somme = %d\n", somme);
	return 0;
}
