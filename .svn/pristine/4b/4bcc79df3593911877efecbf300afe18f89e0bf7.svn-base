/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>



#include <string.h>

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
	va_list va;
	va_start(va, fmt);
	if(_debug >= 0) { // debug verbose mode
		vfprintf(stderr, fmt, va);
	} else { // Don't print out => redirect to /dev/null
		FILE *f = fopen("/dev/null", "a");
		vfprintf(f, fmt, va);
		fclose(f);
	}
	va_end(va);
}
// end of debugging part

#define __STDIO_TEST_STATIC
#ifdef __STDIO_TEST_STATIC
/*ssize_t read(int fd, void *buf, size_t size)
{
	if(size > 30) size = 30;
	memcpy(buf, "012345678901234567890123456789", size);
	sleep(0.1);
	return (ssize_t)size;
} */
#endif // __STDIO_TEST_STATIC

/* Number of iterations */
#define ITER 10
/* Number of threads */
#define NTH 2

typedef union {
        unsigned long long tick;
        struct {
                unsigned low;
                unsigned high;
        };
} tick_t;

int fd[2][2];
sem_t* thread_ready;

#define TICK_DIFF(t1, t2) \
           ((t2).tick - (t1).tick)

#define TIME_DIFF(t1, t2) \
        ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))



/* Fake computation of usec microseconds */
void compute(int usec) {
        struct timeval tv1,tv2;
        gettimeofday(&tv1,NULL);
	do {
		gettimeofday(&tv2,NULL);
	}while(TIME_DIFF(tv1, tv2) < usec);	
}

void* f_thread(void* arg) {
  int my_id= *(int*)arg;

  /* Notify the main thread that we got the args */
  sem_post(thread_ready);

  debug("Running thread #%d\n", my_id);

  int buf = -1;

  int i;
  for(i=0; i<ITER; i++){
    /* Wait until the previous thread has finished his job */
    read(fd[my_id][0], &buf, sizeof(int));

    debug("[thread #%d] read %d\n", my_id, buf);

    /* compute for 1ms */
    compute(50000);

    buf++;
    /* Wake up the next thread */
    write(fd[(my_id+1)%2][1], &buf, sizeof(int));
    buf = 0;
  }

  debug("End of thread #%d\n", my_id);
  return NULL;
}

int main(int argc, char**argv)
{
  pthread_t tid[NTH];
  int i;

  if(pipe(fd[0]) || pipe(fd[1])) {
    fprintf(stderr, "error while calling pipe\n");
    return 1;
  }

  sem_unlink("ready");
  thread_ready = sem_open("ready", O_CREAT, 0666, 0);

  for(i=0;i<NTH; i++)
    {
      pthread_create(&tid[i], NULL, f_thread, &i);
      sem_wait(thread_ready);
    }
  sem_unlink("ready");
  i = 0;
  debug("Main thread writes %d\n", i);
  write(fd[0][1], &i, sizeof(int));
  for(i=0;i<NTH; i++)
    {
      pthread_join(tid[i], NULL);
    }

  return 0;
}

