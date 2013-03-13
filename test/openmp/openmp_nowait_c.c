#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

#define N 100000
#define M 10

int main() {
  int i, j;
  double  a[N], b[N], c[N];
  double r = 0;

  for(i=0; i<N; i++) {
    a[i] = sqrt(i);
    b[i] = 0;
    c[i] = 0;
  }

#pragma omp parallel
  {
#pragma omp for private(i, j) nowait
    for(i=0; i<N; i++) {
      for(j=0; j<((i/10000)+1)*M; j++) {
	b[i] = b[i] + sqrt(a[i])/3;
      }
    }


#pragma omp for private(i, j)
    for(i=0; i<N; i++) {
      for(j=0; j<((i/10000)+1)*M; j++) {
	c[i] = c[i] + sqrt(a[i]);
      }
    }


#pragma omp for private(i, j)
    for(i=0; i<N; i++) {
      for(j=0; j<((i/10000)+1)*M; j++) {
	b[i] = b[i] + sqrt(a[i])/3 + c[i];
      }
    }

#pragma omp barrier

#pragma omp master
    {
      for(i=0; i<N; i++) {
	r = r+ b[i];
      }

      debug("r = %lf\n", r);
    }
  }

  return 0;
}
