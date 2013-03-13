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

#define SIZE (1024*8)

int res = 0;

omp_lock_t lock;

void task_function(int n) {
  //  printf("task_function(%d) : res = %d\n", n, res);
  if(n%2) {
    omp_set_lock(&lock);
    res += 1;
    omp_unset_lock(&lock);
  }
}

int main(void)
{
	int i;

	omp_init_lock(&lock);

	int j;
#pragma omp parallel
	for(j=0;j<10; j++) {
		debug("loop %d\n", j);
 		for(i=0;i<SIZE; i++)
 		{
		  if(j%2) {
#pragma omp task untied
		  task_function(i);
		  } else {
#pragma omp task
		  task_function(i);

		  }
 		}

#pragma omp taskwait
	}

	debug("result = %d\n", res);
	return 0;
}
