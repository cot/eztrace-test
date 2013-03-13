/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1, Telecom SudParis
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
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

/* This program calls functions (more or less) randomly and recursively.
 * At some point, it raises a SIGSEGV.
  */



typedef union {
        unsigned long long tick;
        struct {
                unsigned low;
                unsigned high;
        };
} tick_t;

#define TICK_DIFF(t1, t2) \
           ((t2).tick - (t1).tick)

#define TIME_DIFF(t1, t2) \
        ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

void func1(int a);
void func2(int a);
void func3(int a);

/* Fake computation of usec microseconds */
void compute(int usec) {
        struct timeval tv1,tv2;
        gettimeofday(&tv1,NULL);
	do {
		gettimeofday(&tv2,NULL);
	}while(TIME_DIFF(tv1, tv2) < usec);
}

void func1(int a)
{
  debug("func1(%d)\n", a);
  compute(100*a);

  int alea = rand() % 7;
  if(!alea)
    *(int*)0=0;

  switch(a%3)
    {
    case 0:
      func1(a-1);
      break;
    case 1:
      func2(a-1);
      break;
    case 2:
      func3(a-1);
      break;
    }
}

void func2(int a)
{
  debug("func2(%d)\n", a);
  compute(100*a);

  int alea = rand() % 6;
  if(!alea)
    *(int*)0=0;

  switch(a%3)
    {
    case 0:
      func1(a-1);
      break;
    case 1:
      func2(a-1);
      break;
    case 2:
      func3(a-1);
      break;
    }
}

void func3(int a)
{
  debug("func3(%d)\n", a);
  compute(100*a);

  int alea = rand() % 5;
  if(!alea)
    *(int*)0=0;

  switch(a%3)
    {
    case 0:
      func1(a-1);
      break;
    case 1:
      func2(a-1);
      break;
    case 2:
      func3(a-1);
      break;
    }
}

int main(int argc, char**argv)
{
  srand(time(NULL));
  while(1)
    func1(rand()%101);
  return 0;
}
