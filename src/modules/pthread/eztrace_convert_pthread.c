/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <GTG.h>
#include <assert.h>

#include "eztrace_convert.h"
#include "eztrace_convert_core.h"
#include "pthread_ev_codes.h"
#include "eztrace_list.h"
#include "eztrace_stack.h"
#include "eztrace_convert_pthread.h"

/* Semaphore processing */
void
handle_sem_post (void)
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);

  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, SEMAPHORE);

  PTHREAD_CHANGE() addEvent(CURRENT, "E_LockStart", thread_id, lock_info->info);
}

void
handle_start_sem_wait (void)
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);

  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, SEMAPHORE);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked_sem");
}

void
handle_stop_sem_wait (void)
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);

  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, SEMAPHORE);

  lock_info->nb_acquire++;
  lock_info->last_owner_tid = CUR_THREAD_ID;

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
  PTHREAD_CHANGE() addEvent(CURRENT, "E_SemWait_Done", thread_id, lock_info->info);
}


/* Spinlock processing */
void
handle_spin_lock_start ()
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);

  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, SPINLOCK);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked_spin");
}

void
handle_spin_lock_stop ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);

  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, SPINLOCK);
  record_lock_acquired(lock_info);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Critical_spin");
}

void
handle_spin_trylock ()
{
  FUNC_NAME;
  int result = GET_PARAM(CUR_EV, 2);
  if(result) {
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    DECLARE_CUR_PROCESS(p_process);

    app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
    struct pthread_lock_info_t *lock_info = NULL;
    GET_LOCK_INFO(lock_info, p_process, lock_ptr, SPINLOCK);
    record_lock_acquired(lock_info);

    PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Critical_spin");
  }
}

void
handle_spin_unlock ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, SPINLOCK);

  record_lock_release(lock_info);
  __check_lock_mismatch(lock_info, p_process, CUR_THREAD_ID);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}


/* Mutex processing */
void
handle_mutex_trylock (int result)
{
  FUNC_NAME;
  if (!result)
    return;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, MUTEX);
  record_lock_acquired(lock_info);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Critical_mutex");
}

void
handle_mutex_lock_start ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, MUTEX);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked_mutex");
}

void
handle_mutex_lock_stop ()
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, MUTEX);
  record_lock_acquired(lock_info);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Critical_mutex");
}

void
handle_mutex_unlock ()
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, MUTEX);
  record_lock_release(lock_info);
  __check_lock_mismatch(lock_info, p_process, CUR_THREAD_ID);

  PTHREAD_CHANGE() addEvent(CURRENT, "E_Mutex_Unlock", thread_id, lock_info->info);
  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}


/* Condition processing */
void
handle_cond_signal ()
{
  FUNC_NAME;

  //  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  //  DECLARE_CUR_THREAD(p_thread);

  //  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  //  struct pthread_lock_info_t *lock_info = NULL;
  //  GET_LOCK_INFO(lock_info, p_thread, lock_ptr, MUTEX);
}

void
handle_cond_broadcast ()
{
  FUNC_NAME;
}

void
handle_cond_start_wait ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked_cond");
}

void
handle_cond_stop_wait ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, CONDITION);
  record_lock_acquired(lock_info);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}


/* RWLock processing */
void
handle_rwlock_rdlock_start ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, RWLOCK);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked_rwlock");
}

void
handle_rwlock_rdlock_stop ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);

  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, RWLOCK);
  record_lock_acquired(lock_info);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Critical_rwlock");
}

void
handle_rwlock_wrlock_start ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, RWLOCK);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked_rwlock");
}

void
handle_rwlock_wrlock_stop ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);

  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, RWLOCK);
  record_lock_acquired(lock_info);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Critical_rwlock");
}

void
handle_rwlock_unlock ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  DECLARE_CUR_PROCESS(p_process);

  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct pthread_lock_info_t *lock_info = NULL;
  GET_LOCK_INFO(lock_info, p_process, lock_ptr, RWLOCK);

  record_lock_release(lock_info);
  __check_lock_mismatch(lock_info, p_process, CUR_THREAD_ID);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}


void
handle_barrier_start ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

//  DECLARE_CUR_THREAD(p_thread);
//  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
//  struct pthread_lock_info_t *lock_info = NULL;
//  GET_LOCK_INFO(lock_info, p_thread, lock_ptr, BARRIER);

  PTHREAD_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked_barrier");
}

void
handle_barrier_stop ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

//  DECLARE_CUR_THREAD(p_thread);
//  app_ptr lock_ptr = (app_ptr) GET_PARAM(CUR_EV, 1);
//  struct pthread_lock_info_t *lock_info = NULL;
//  GET_LOCK_INFO(lock_info, p_thread, lock_ptr, BARRIER);

  PTHREAD_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}

int
eztrace_convert_pthread_init()
{
  if(get_mode() == EZTRACE_CONVERT) {
    addEventType("E_SemPost", "CT_Thread", "SemPost");
    addEventType("E_SemWait_Done", "CT_Thread", "SemWait done");

    addEventType("E_Mutex_Lock", "CT_Thread", "Mutex Lock");
    addEventType("E_Mutex_Unlock", "CT_Thread", "Mutex Unlock");

    addEventType("E_CondSignal", "CT_Thread", "Cond Signal");

    addEventType("E_LockStart", "CT_Thread", "Waiting for a lock");
    addEventType("E_LockStop", "CT_Thread", "Lock acquired");

    addEntityValue("STV_Blocked_sem", "ST_Thread", "Blocked on a semaphore", GTG_RED);
    addEntityValue("STV_Blocked_spin", "ST_Thread", "Blocked on a spinlock", GTG_RED);
    addEntityValue("STV_Blocked_mutex", "ST_Thread", "Blocked on a mutex", GTG_RED);
    addEntityValue("STV_Blocked_condition", "ST_Thread", "Blocked on a condition", GTG_RED);
    addEntityValue("STV_Blocked_rwlock", "ST_Thread", "Blocked on a rwlock", GTG_RED);
    addEntityValue("STV_Blocked_barrier", "ST_Thread", "Blocked on a barrier", GTG_RED);

    addEntityValue("STV_Critical_sem", "ST_Thread", "Critical Section (semaphore)", GTG_GREEN);
    addEntityValue("STV_Critical_spin", "ST_Thread", "Critical Section (spinlock)", GTG_GREEN);
    addEntityValue("STV_Critical_mutex", "ST_Thread", "Critical Section (mutex)", GTG_GREEN);
    addEntityValue("STV_Critical_condition", "ST_Thread", "Critical Section (condition)", GTG_GREEN);
    addEntityValue("STV_Critical_rwlock", "ST_Thread", "Critical Section (rwlock)", GTG_GREEN);
  }
  return 0;
}

/* return 1 if the event was handled */
int
handle_pthread_events(struct fxt_ev_64 *ev)
{
  if(!STARTED)
    return 0;

  switch(ev->code)
    {
      /* PThread creation/destruction */

      /* Semaphore */
    case FUT_SEM_POST:
      handle_sem_post ();
      break;
    case FUT_SEM_START_WAIT:
      handle_start_sem_wait ();
      break;
    case FUT_SEM_STOP_WAIT:
      handle_stop_sem_wait ();
      break;

      /* Spinlock */
    case FUT_SPIN_LOCK_START:
      handle_spin_lock_start ();
      break;
    case FUT_SPIN_LOCK_STOP:
      handle_spin_lock_stop ();
      break;
    case FUT_SPIN_TRYLOCK:
      handle_spin_trylock ();
      break;
    case FUT_SPIN_UNLOCK:
      handle_spin_unlock ();
      break;

      /* Mutex */
    case FUT_MUTEX_TRYLOCK_SUCCESS:
      handle_mutex_trylock (1);
      break;
    case FUT_MUTEX_TRYLOCK_FAIL:
      handle_mutex_trylock (0);
      break;
    case FUT_MUTEX_LOCK_START:
      handle_mutex_lock_start ();
      break;
    case FUT_MUTEX_LOCK_STOP:
      handle_mutex_lock_stop ();
      break;
    case FUT_MUTEX_UNLOCK:
      handle_mutex_unlock ();
      break;

      /* Condition */
    case FUT_COND_SIGNAL:
      handle_cond_signal ();
      break;
    case FUT_COND_BROADCAST:
      handle_cond_broadcast ();
      break;
    case FUT_COND_START_WAIT:
      handle_cond_start_wait ();
      break;
    case FUT_COND_STOP_WAIT:
      handle_cond_stop_wait ();
      break;

      /* RWLock */
    case FUT_RWLOCK_RDLOCK_START:
      handle_rwlock_rdlock_start ();
      break;
    case FUT_RWLOCK_RDLOCK_STOP:
      handle_rwlock_rdlock_stop ();
      break;
    case FUT_RWLOCK_WRLOCK_START:
      handle_rwlock_wrlock_start();
      break;
    case FUT_RWLOCK_WRLOCK_STOP:
      handle_rwlock_wrlock_stop();
      break;
    case FUT_RWLOCK_UNLOCK:
      handle_rwlock_unlock();
      break;

    case FUT_BARRIER_START:
      handle_barrier_start();
      break;
    case FUT_BARRIER_STOP:
      handle_barrier_stop();
      break;
    default:
      return 0;
    }
  return 1;
}

int
handle_pthread_stats(struct fxt_ev_64 *ev)
{
  recording_stats = 1;
  return handle_pthread_events(ev);
}

void print_pthread_stats() {
  printf ( "\nPThread:\n");
  printf   ( "-------\n");
  /* todo:
   * add:
   *  - min/max/average duration of critical sections
   *  - min/max/average delay to get a lock
   */

  int i;
  for(i=0; i<NB_TRACES; i++) {
    struct process_info_t *p_process = GET_PROCESS_INFO(i);
    struct pthread_process_info_t *p_info = (struct pthread_process_info_t*) ezt_hook_list_retrieve_data(&p_process->hooks, (uint8_t)PTHREAD_EVENTS_ID);

    if(!p_info)
      continue;			/* No gomp process info attached, skip this process */


    uint32_t nb_acquire = 0;
    uint32_t nb_locks = 0;

    printf("%s:\n", p_process->container->name);
    ezt_stack_token_t *token;
    ezt_list_foreach(&p_info->lock_list, token) {
      struct pthread_lock_info_t * lock = (struct pthread_lock_info_t *) token->data;
      nb_acquire += lock->nb_acquire;
      nb_locks++;
      switch(lock->lock) {
      case MUTEX:
	printf("\tmutex");
	break;
      case SPINLOCK:
	printf("\tspinlock");
	break;
      case RWLOCK:
	printf("\trwlock");
	break;
      case SEMAPHORE:
	printf("\tsemaphore");
	break;
      case BARRIER:
	printf("\tbarrier");
	break;
      case CONDITION:
	printf("\tcondition");
	break;
      };
      printf(" 0x%x was acquired %d times.", lock->ptr, lock->nb_acquire);
      if(lock->duration_critical_section)
	printf(" total duration of critical_sections: %lf ms.", lock->duration_critical_section);
      printf("\n");
    }
    printf("Total: %d locks acquired %d times\n", nb_locks, nb_acquire);

    __free_lock_info_list(p_info);
  }
}

struct eztrace_convert_module pthread_module;

void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  pthread_module.api_version = EZTRACE_API_VERSION;
  pthread_module.init = eztrace_convert_pthread_init;
  pthread_module.handle = handle_pthread_events;
  pthread_module.handle_stats = handle_pthread_stats;
  pthread_module.print_stats = print_pthread_stats;

  pthread_module.module_prefix = PTHREAD_EVENTS_ID;
  asprintf(&pthread_module.name, "pthread");
  asprintf(&pthread_module.description, "Module for PThread synchronization functions (mutex, semaphore, spinlock, etc.)");

  pthread_module.token.data = &pthread_module;
  eztrace_convert_register_module(&pthread_module);
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
}
