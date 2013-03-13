/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE 1 /* or _BSD_SOURCE or _SVID_SOURCE */
#define _REENTRANT
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "pthread_ev_codes.h"
#include "eztrace.h"
#include "pptrace.h"

/* set to 1 when all the hooks are set.
 * This is usefull in order to avoid recursive calls to mutex_lock for example
 */
static volatile int __pthread_initialized = 0;

/* pointers to actual pthread functions */
int  (*libpthread_mutex_lock) (pthread_mutex_t * mutex) = NULL;
int  (*libpthread_mutex_trylock) (pthread_mutex_t * mutex);
int  (*libpthread_mutex_unlock) (pthread_mutex_t * mutex);

int  (*libpthread_spin_lock) (pthread_spinlock_t * lock);
int  (*libpthread_spin_unlock) (pthread_spinlock_t * lock);
int  (*libpthread_spin_trylock) (pthread_spinlock_t * lock);

int  (*libpthread_barrier_wait) (pthread_barrier_t * barrier);

int  (*libpthread_sem_wait) (sem_t * sem);
int  (*libpthread_sem_post) (sem_t * sem);

int  (*libpthread_cond_init) (pthread_cond_t *__restrict __cond,
                              __const pthread_condattr_t *__restrict
                              __cond_attr);

int  (*libpthread_cond_wait) (pthread_cond_t * __restrict cond,
			      pthread_mutex_t * __restrict mutex);
int  (*libpthread_cond_broadcast) (pthread_cond_t * cond);
int  (*libpthread_cond_signal) (pthread_cond_t * cond);
int  (*libpthread_cond_destroy) (pthread_cond_t *__cond);

int  (*libpthread_cond_timedwait) (pthread_cond_t *__restrict __cond,
                                   pthread_mutex_t *__restrict __mutex,
                                   __const struct timespec *__restrict
                                   __abstime);

int  (*libpthread_rwlock_rdlock) (pthread_rwlock_t * rwlock);
int  (*libpthread_rwlock_wrlock) (pthread_rwlock_t * rwlock);
int  (*libpthread_rwlock_unlock) (pthread_rwlock_t * rwlock);

/* Mutex-related callbacks */
int
pthread_mutex_lock (pthread_mutex_t * mutex)
{
  FUNCTION_ENTRY;
  /* If the current file's constructor has not been called yet, 
     this means that the application is being initialized and that 
     there's only one thread. Thus, we do not to call mutex_lock for 
     real.
     If you try to call mutex_lock in that case, you'll get a SIGSEVG 
     since dlsym was not called yet for mutex_lock
  */
  if(__pthread_initialized) {
    EZTRACE_PROTECT EZTRACE_EVENT1(FUT_MUTEX_LOCK_START, mutex);
    int ret = libpthread_mutex_lock(mutex);
    EZTRACE_PROTECT EZTRACE_EVENT2(FUT_MUTEX_LOCK_STOP, mutex, ret);
    return ret;
  }
  /* __pthread_initialized is set to 0. The application is initializing, 
     so don't do anything 
  */
  return 0;
}

int
pthread_mutex_trylock (pthread_mutex_t * mutex)
{
  FUNCTION_ENTRY;
  if(__pthread_initialized) {
    int ret = libpthread_mutex_trylock(mutex);

    /* change #if 0 into #if 1 if you want to record each call to mutex_trylock
       (ie. not only successful calls) */
#if 0
    EZTRACE_PROTECT EZTRACE_EVENT1(ret==1 ? FUT_MUTEX_TRYLOCK_FAIL : FUT_MUTEX_TRYLOCK_SUCCESS, mutex);
#else
    if(!ret){
      EZTRACE_PROTECT EZTRACE_EVENT1(FUT_MUTEX_TRYLOCK_SUCCESS, mutex);
    }
#endif
    return ret;
  }
  return 0;
}

int 
pthread_mutex_unlock (pthread_mutex_t * mutex)
{
  FUNCTION_ENTRY;
  if(__pthread_initialized) {
    EZTRACE_PROTECT EZTRACE_EVENT1(FUT_MUTEX_UNLOCK, mutex);
    int ret = libpthread_mutex_unlock(mutex);
    if(ret)
      fprintf(stderr, "pthread_mutex_lock(mutex=%p) returned %d\n", mutex, ret);
    return ret;
  }
  return 0;
}


/* Spinlock-related callbacks */
int
pthread_spin_lock (pthread_spinlock_t * lock)
{
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_SPIN_LOCK_START, lock);
  int ret = libpthread_spin_lock(lock);
  EZTRACE_PROTECT EZTRACE_EVENT2(FUT_SPIN_LOCK_STOP, lock, ret);
  return ret;
}

int
pthread_spin_unlock (pthread_spinlock_t * lock)
{
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_SPIN_UNLOCK, lock);
  return libpthread_spin_unlock(lock);
}

int
pthread_spin_trylock (pthread_spinlock_t * lock)
{
  FUNCTION_ENTRY;
  int ret = libpthread_spin_trylock(lock);
  EZTRACE_PROTECT EZTRACE_EVENT2(FUT_SPIN_TRYLOCK, lock, ret);
  return ret;
}


/* Barrier-related callbacks */
int
pthread_barrier_wait (pthread_barrier_t * barrier)
{
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_BARRIER_START, barrier);
  int ret = libpthread_barrier_wait(barrier);
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_BARRIER_STOP, barrier);
  return ret;
}


/* Semaphore-related callbacks */
int
sem_wait (sem_t * sem)
{
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_SEM_START_WAIT, sem);
  int ret = libpthread_sem_wait(sem);
  EZTRACE_PROTECT EZTRACE_EVENT2(FUT_SEM_STOP_WAIT, sem, ret);
  return ret;
}

int
sem_post (sem_t * sem)
{
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_SEM_POST, sem);
  return libpthread_sem_post(sem);
}

/* Condition-related callbacks */

/* We don't actually need these (init, destroy) functions, but if we don't 
 * catch these functions, it will lead to using somehow different implementations
 * (ie. cond_init from one implementation and cond_wait from another) that are 
 * not compatible, resulting in data corruption :(
 */
int
pthread_cond_init (pthread_cond_t *__restrict __cond,
		   __const pthread_condattr_t *__restrict __cond_attr)
{
  if(!libpthread_cond_init) {
    INTERCEPT("pthread_cond_init", libpthread_cond_init);
  }
  return libpthread_cond_init(__cond, __cond_attr);
}

int
pthread_cond_destroy (pthread_cond_t *__cond)
{
  if(!libpthread_cond_destroy) {
    INTERCEPT("pthread_cond_destroy", libpthread_cond_destroy);
  }
  return libpthread_cond_destroy(__cond);
}

int
pthread_cond_wait (pthread_cond_t * __restrict cond, pthread_mutex_t * __restrict mutex)
{
  if(!libpthread_cond_wait) {
    INTERCEPT("pthread_cond_wait", libpthread_cond_wait);
  }
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT2(FUT_COND_START_WAIT, cond, mutex);
  int ret = libpthread_cond_wait(cond, mutex);
  EZTRACE_PROTECT EZTRACE_EVENT3(FUT_COND_STOP_WAIT, cond, mutex, ret);
  return ret;
}

int
pthread_cond_timedwait (pthread_cond_t *__restrict __cond,
			pthread_mutex_t *__restrict __mutex,
			__const struct timespec *__restrict __abstime)
{
  if(!libpthread_cond_timedwait) {
    INTERCEPT("pthread_cond_timedwait", libpthread_cond_timedwait);
  }
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT3(FUT_COND_START_WAIT, __cond, __mutex, __abstime);
  int ret = libpthread_cond_timedwait(__cond, __mutex, __abstime);
  /* todo: what if the timer expires ? */
  EZTRACE_PROTECT EZTRACE_EVENT3(FUT_COND_STOP_WAIT, __cond, __mutex, ret);
  return ret;
}

int
pthread_cond_broadcast (pthread_cond_t * cond)
{
  if(!libpthread_cond_broadcast) {
    INTERCEPT("pthread_cond_broadcast", libpthread_cond_broadcast);
  }
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_COND_BROADCAST, cond);
  return libpthread_cond_broadcast(cond);
}


int
pthread_cond_signal (pthread_cond_t * cond)
{
  if(!libpthread_cond_signal) {
    INTERCEPT("pthread_cond_signal", libpthread_cond_signal);
  }

  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_COND_SIGNAL, cond);
  return libpthread_cond_signal(cond);
}


/* RWLock-related callbacks */
int
pthread_rwlock_rdlock (pthread_rwlock_t * rwlock)
{
  if(!libpthread_rwlock_rdlock) {
    INTERCEPT("pthread_rwlock_rdlock", libpthread_rwlock_rdlock);
  }
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_RWLOCK_RDLOCK_START, rwlock);
  int ret = libpthread_rwlock_rdlock(rwlock);
  EZTRACE_PROTECT EZTRACE_EVENT2(FUT_RWLOCK_RDLOCK_STOP, rwlock, ret);
  return ret;
}

int pthread_rwlock_wrlock (pthread_rwlock_t * rwlock)
{
  if(!libpthread_rwlock_wrlock) {
    INTERCEPT("pthread_rwlock_wrlock", libpthread_rwlock_wrlock);
  }
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_RWLOCK_WRLOCK_START, rwlock);
  int ret = libpthread_rwlock_wrlock(rwlock);
  EZTRACE_PROTECT EZTRACE_EVENT2(FUT_RWLOCK_WRLOCK_STOP, rwlock, ret);
  return ret;
}

int pthread_rwlock_unlock(pthread_rwlock_t * rwlock)
{
  if(!libpthread_rwlock_unlock) {
    INTERCEPT("pthread_rwlock_unlock", libpthread_rwlock_unlock);
  }
  FUNCTION_ENTRY;
  EZTRACE_PROTECT EZTRACE_EVENT1(FUT_RWLOCK_UNLOCK, rwlock);
  int ret = libpthread_rwlock_unlock(rwlock);
  return ret;
}

START_INTERCEPT
   INTERCEPT2("pthread_mutex_lock", libpthread_mutex_lock)
   INTERCEPT2("pthread_mutex_trylock", libpthread_mutex_trylock)
   INTERCEPT2("pthread_mutex_unlock", libpthread_mutex_unlock)

   INTERCEPT2("pthread_spin_lock", libpthread_spin_lock)
   INTERCEPT2("pthread_spin_unlock", libpthread_spin_unlock)
   INTERCEPT2("pthread_spin_trylock", libpthread_spin_trylock)
   INTERCEPT2("pthread_barrier_wait", libpthread_barrier_wait)
   
   INTERCEPT2("sem_wait", libpthread_sem_wait)
   INTERCEPT2("sem_post", libpthread_sem_post)

   INTERCEPT2("pthread_cond_wait", libpthread_cond_wait)
   INTERCEPT2("pthread_cond_timedwait", libpthread_cond_timedwait)
   INTERCEPT2("pthread_cond_broadcast", libpthread_cond_broadcast)
   INTERCEPT2("pthread_cond_signal", libpthread_cond_signal)
   INTERCEPT2("pthread_cond_init", libpthread_cond_init)
   INTERCEPT2("pthread_cond_destroy", libpthread_cond_destroy)

   INTERCEPT2("pthread_rwlock_rdlock", libpthread_rwlock_rdlock)
   INTERCEPT2("pthread_rwlock_wrlock", libpthread_rwlock_wrlock)
   INTERCEPT2("pthread_rwlock_unlock", libpthread_rwlock_unlock)
END_INTERCEPT

static void __pthread_init (void) __attribute__ ((constructor));
static void
__pthread_init (void)
{
  DYNAMIC_INTERCEPT_ALL();
#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#endif
  __pthread_initialized = 1;
}

static void __pthread_conclude (void) __attribute__ ((destructor));
static void
__pthread_conclude (void)
{
  __pthread_initialized = 0;
  eztrace_stop ();
}
