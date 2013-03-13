/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
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

int main()
{
  pthread_cond_t cond;
  pthread_mutex_t mutex;

  debug("Testing pthread_cond_*\n");

  pthread_cond_init(&cond, NULL);
  pthread_mutex_init(&mutex, NULL);

  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&cond);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);

  pthread_cond_destroy(&cond);
  debug("OK ! \n");
  return 0;
}
