#include "eztrace_convert.h"
#include <inttypes.h>

static int recording_stats = 0;
static double duration_critical_section = 0.0; // total duration of critical sections
/* todo:
 * - this should be computed for each process
 * - for now, this records the cumulated duration of critical section. If a thread calls
 *   mutex_lock(l1); mutex_lock(l2); the recorded duration of the critical section is twice
 *   the time spent holding l1.
 */

#define PTHREAD_CHANGE() if(!recording_stats) CHANGE()

/* structure attached to each process */
struct pthread_process_info_t {
  struct process_info_t *p_process; /* pointer to the corresponding process */
  struct ezt_list_t lock_list;	    /* list of struct pthread_lock_info_t */
};

enum lock_type_t{
  MUTEX,
  SPINLOCK,
  RWLOCK,
  SEMAPHORE,
  BARRIER,
  CONDITION
};

/* structure that contains information on a lock(mutex, spinloc, etc.)
 */
struct pthread_lock_info_t {
  struct ezt_list_token_t token; /* token used for inserting this structure in the lock_list */
  enum lock_type_t lock;	 /* type of lock */
  app_ptr ptr;			 /* address of the lock in the application */
  char* info;			 /* name of the lock */
  /* statistics: */
  uint32_t nb_acquire; /* number of lock_stop */
  uint64_t last_owner_tid; /* tid of the last thread that acquired the lock */
  float time_acquire; /* last time the lock was acquired */
  float time_release; /* last time the lock was released */
  double duration_critical_section; /* time spent in critical section */
  /* todo: improve statistics:
   * - compute min/max/average critical section duration
   * - record the time spent waiting for the lock
   */
};

/* print a message that warns about a dangerous behavior. Thread 1 locked, but thread2 unlocked.
 * return 1 if this behavior is detected or 0 otherwise
 */
static int __check_lock_mismatch(struct pthread_lock_info_t *p_info,
			    struct process_info_t *p_process,
			    uint64_t cur_tid)
{
  if(p_info->last_owner_tid == cur_tid)
    return 0;

  printf("Warning: t=%f - %s\tthread %"PRIu64" locked ", CURRENT,
	 p_process->container->id, p_info->last_owner_tid);
  switch (p_info->lock) {
  case MUTEX:
    printf("mutex");
    break;
  case SPINLOCK:
    printf("spinlock");
    break;
  case RWLOCK:
    printf("rwlock");
    break;
  case SEMAPHORE:
    printf("semaphore");
    break;
  case BARRIER:
    printf("barrier");
    break;
  case CONDITION:
    printf("condition");
    break;
  };
  printf(" %p, but thread %"PRIu64" unlocks it!\n",p_info->ptr, cur_tid);
  return 1;
}

/* add a hook in the process structure in order to store information
 * about locks used by the process
 */
static struct pthread_process_info_t *__register_process_hook(struct process_info_t *p_process)
{
  struct pthread_process_info_t *p_info = (struct pthread_process_info_t*) malloc(sizeof(struct pthread_process_info_t));
  p_info->p_process = p_process;
  ezt_list_new(&p_info->lock_list);

  /* add the hook in the process info structure */
  ezt_hook_list_add(&p_info->p_process->hooks, p_info, (uint8_t)PTHREAD_EVENTS_ID);
  return p_info;
}


/* declare a pthread_process_info_t that corresponds to p_process
 */
#define  INIT_PTHREAD_PROCESS_INFO(p_process, var)			\
  struct pthread_process_info_t *var = (struct pthread_process_info_t*)	\
    ezt_hook_list_retrieve_data(&p_process->hooks, (uint8_t)PTHREAD_EVENTS_ID); \
  if(!(var)) {								\
    var = __register_process_hook(p_process);				\
  }


/* get the pthread_lock_info_t* lock_info that corresponds to lock_ptr
 * This initialize lock_info->info
 */
#define GET_LOCK_INFO(lock_info, p_process, lock_ptr, lock_type)	\
  do {									\
    INIT_PTHREAD_PROCESS_INFO(p_process, p_info);			\
    lock_info = __find_lock_info(&p_info->lock_list, lock_ptr);		\
    if(!lock_info) {							\
      lock_info = __new_lock_info(p_info, lock_ptr, lock_type);		\
    }									\
  }while(0)


/* add the current duration of critical section (time_release-time_acquire) to the
 * sum of critical sections
 */
#define update_duration_critical_section(p_lock)			\
  do {									\
    p_lock->duration_critical_section += p_lock->time_release - p_lock->time_acquire; \
    duration_critical_section += p_lock->time_release - p_lock->time_acquire; \
  } while(0)


/* record informations on a lock acquire event (for statistics) */
#define record_lock_acquired(p_lock)		\
  do {						\
    p_lock->time_acquire = CURRENT;		\
    p_lock->nb_acquire++;			\
    p_lock->last_owner_tid = CUR_THREAD_ID;	\
  } while(0)

/* record informations on a lock release event (for statistics) */
#define record_lock_release(p_lock)		\
  do {						\
    p_lock->time_release = CURRENT;		\
    update_duration_critical_section(p_lock);	\
  }while(0)

/* Free the lock_info_list attached to a process
 * Since this also frees the locks info strings, this should not be called before
 * the trace is written to disk.
 */
static void __free_lock_info_list(struct pthread_process_info_t* p_info) {

  while(!ezt_list_empty(&p_info->lock_list)) {
    struct ezt_list_token_t *t = ezt_list_get_head(&(p_info->lock_list));
    struct pthread_lock_info_t *p = (struct pthread_lock_info_t *) t->data;

    ezt_list_remove(t);

    if(p) {
      free(p->info);
      free(p);
    }
  }
}

/* create a new lock_info structure */
static struct pthread_lock_info_t*
__new_lock_info(struct pthread_process_info_t* p_info,
		app_ptr ptr,
		enum lock_type_t lock_type) {
  struct pthread_lock_info_t* res = malloc(sizeof(struct pthread_lock_info_t));
  /* initialize the structure */
  res->lock = lock_type;
  res->ptr = ptr;

  switch(lock_type) {
  case MUTEX:
    asprintf(&res->info, "Mutex %p.", ptr);
    break;

  case SPINLOCK:
    asprintf(&res->info, "Spinlock %p.", ptr);
    break;

  case RWLOCK:
    asprintf(&res->info, "RWLock %p.", ptr);
    break;

  case SEMAPHORE:
    asprintf(&res->info, "Semaphore %p.", ptr);
    break;

  case BARRIER:
    asprintf(&res->info, "Barrier %p.", ptr);
    break;

  default:
    fprintf(stderr, "unknown lock type: %d\n", lock_type);
  }

  asprintf(&res->info, "%s_ptr_%p", p_info->p_process->container->id, ptr);

  /* initialize statistics */
  res->nb_acquire = 0;
  res->last_owner_tid = -1;
  res->time_acquire = 0;
  res->time_release = 0;
  res->duration_critical_section = 0;

  res->token.data = res;

  /* add the lock to the list of locks */
  ezt_list_add(&p_info->lock_list, &res->token);

  return res;
}

/* find the lock_info that corresponds to ptr.
 * return NULL if not found
 */
static struct pthread_lock_info_t*__find_lock_info(struct ezt_list_t *lock_list, app_ptr ptr) {
  struct ezt_list_token_t *t = NULL;
  struct pthread_lock_info_t *p;
  ezt_list_foreach(lock_list, t){
    p = (struct pthread_lock_info_t *) t->data;
    if(p->ptr == ptr)
      return p;
  }
  return NULL;
}

