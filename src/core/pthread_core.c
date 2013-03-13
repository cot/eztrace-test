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

#include "ev_codes.h"
#include "eztrace.h"

//#define VERBOSE 1
#ifdef VERBOSE
#define FUNCTION_NAME fprintf(stderr, "Calling [%s]\n", __FUNCTION__)
#else
#define FUNCTION_NAME (void) 0
#endif

static volatile int __pthread_core_initialized = 0;

int  (*libpthread_create) (pthread_t * thread, pthread_attr_t * attr,
			   void *(*start_routine) (void *), void *arg);
int  (*libpthread_join) (pthread_t th, void **thread_return);
void (*libpthread_exit) (void *thread_return);

/* Thread creation/destruction callbacks */

/* Internal structure used for transmitting the function and argument 
 * during pthread_create.
 */
struct __pthread_create_info_t
{
  void *(*func) (void *);
  void *arg;
};

/* Invoked by pthread_create on the new thread */
static void *
__pthread_new_thread (void *arg)
{
  struct __pthread_create_info_t *p_arg = (struct __pthread_create_info_t*) arg;
  void *(*f) (void *) = p_arg->func;
  void *__arg = p_arg->arg;
  free(p_arg);
  EZTRACE_EVENT0 (FUT_NEW_THREAD);
  RECORD_HW_COUNTERS();
  void *res = (*f)(__arg);
  EZTRACE_EVENT0 (FUT_END_THREAD);
  return res;
}

int
pthread_create (pthread_t *__restrict thread,
			   __const pthread_attr_t *__restrict attr,
			   void *(*start_routine) (void *),
			   void *__restrict arg)
{
  FUNCTION_NAME;
  struct __pthread_create_info_t * __args = (struct __pthread_create_info_t*) malloc(sizeof(struct __pthread_create_info_t));
  __args->func = start_routine;
  __args->arg = arg;

  if(!libpthread_create)
    INTERCEPT("pthread_create", libpthread_create);

  /* We do not call directly start_routine since we could not get the actual creation timestamp of 
   * the thread. Let's invoke __pthread_new_thread that will PROF_EVENT the thread and call 
   * start_routine.
   */
  return libpthread_create(thread, attr, __pthread_new_thread, __args);
}

int 
pthread_join (pthread_t th, void **thread_return)
{
  FUNCTION_NAME;
  EZTRACE_EVENT0(FUT_START_THREAD_JOIN);
  int ret = libpthread_join(th, thread_return);
  EZTRACE_EVENT3(FUT_STOP_THREAD_JOIN, th, thread_return, ret);
  return ret;
}

void 
pthread_exit (void *thread_return)
{
  FUNCTION_NAME;
  libpthread_exit (thread_return);
}


static void __pthread_core_init (void) __attribute__ ((constructor));
static void
__pthread_core_init (void)
{
  INTERCEPT("pthread_create", libpthread_create);
  INTERCEPT("pthread_join", libpthread_join);
  INTERCEPT("pthread_exit", libpthread_exit);

#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#endif
  __pthread_core_initialized = 1;
}

static void __pthread_core_conclude (void) __attribute__ ((destructor));
static void
__pthread_core_conclude (void)
{
  __pthread_core_initialized = 0;
  eztrace_stop ();
}
