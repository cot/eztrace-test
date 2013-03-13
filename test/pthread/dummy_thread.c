/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
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

#define TICK_DIFF(t1, t2) \
           ((t2).tick - (t1).tick)

#define TIME_DIFF(t1, t2) \
        ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))



/* Only used during thread creating to make sure that the thread
 * got the correct args.
 */
sem_t thread_ready;

/* Block/unblock a thread */
sem_t sem[NTH];

pthread_mutex_t mutex;

/* Fake computation of usec microseconds */
void compute(int usec) {
        struct timeval tv1,tv2;
        gettimeofday(&tv1,NULL);
	do {
		gettimeofday(&tv2,NULL);
	}while(TIME_DIFF(tv1, tv2) < usec);	
}

void* f_thread(void* arg) {
	uint8_t my_id=*(uint8_t*) arg;

	/* Notify the main thread that we got the args */
	sem_post(&thread_ready);

	debug("Running thread #%d\n", my_id);

	int i;
	for(i=0; i<ITER; i++){		
		/* Wait until the previous thread has finished his job */
		sem_wait(&sem[my_id]);
		debug("[thread #%d] loop %d\n", my_id, i);

		/* compute for 1ms */
		pthread_mutex_lock(&mutex);
		compute(50000);
		pthread_mutex_unlock(&mutex);

		compute(50000);

		/* Wake up the next thread */
		sem_post(&sem[(my_id+1)%NTH]);
	}

	debug("End of thread #%d\n", my_id);
	return NULL;
}

int main(int argc, char**argv)
{
  pthread_t tid[NTH];
  int i;

  pthread_mutex_init(&mutex, NULL);

  sem_init(&thread_ready, 0, 0);
  for(i=0 ; i<NTH; i++)
    sem_init(&sem[i], 0, 0);
  
  for(i=0;i<NTH; i++) 
    {
      pthread_create(&tid[i], NULL, f_thread, &i);
      sem_wait(&thread_ready);
    }

  /* Unblock the first thread so that it can start working */
  sem_post(&sem[0]);
  
  for(i=0;i<NTH; i++) 
    {
      pthread_join(tid[i], NULL);
    }

  return 0;
}
