#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

#define microsleep(us) do { struct timespec tm; tm.tv_sec = (time_t)us / 1000; tm.tv_nsec = (long)(us % 1000) * 1000; nanosleep(&tm, NULL); } while(0)

int mafunc(int n) {
   debug("i sleep %d microsec\n", n);
   microsleep(n);
   return EXIT_SUCCESS;
}

int main() {
   int i=0;
   debug("hello i m in main an i will wait 4 microsec\n");
   microsleep(4);
   for (i=0; i<4; i++) {
      mafunc(i);
      debug("main sleep 1 microsec\n");
      microsleep(1);
   }
   return EXIT_SUCCESS;
   
}
