/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _SVID_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <GTG.h>
#include <dirent.h>
#include <dlfcn.h>
#include <assert.h>

#include "eztrace_convert.h"
#include "eztrace_convert_core.h"
#include "eztrace_convert_types.h"
#include "ev_codes.h"

struct fxt_ev_64 ev;

/* a thread just processed the current event, let's handl ethe next one */
sem_t job_processed;

/* set to 1 if verbose */
int verbose = 0;

/* number of traces currently loaded */
int nb_traces = 0;
/* number of traces currently started (ie. that have reached MPI_Init) */
int nb_start = 0;
/* array that contains the loadedtraces */
struct trace_t *traces = NULL;
/* pointer to the trace being processed */
struct trace_t *cur_trace = NULL;
/* pointer to the event being processed */
struct fxt_ev_64 *cur_ev = NULL;

struct eztrace_event_handler __handler_info;
pthread_t *main_thread;
sem_t main_thread_sem;

static enum eztrace_mode __cur_mode = EZTRACE_CONVERT;

void eztrace_convert_init(unsigned __nb_traces)
{
  if(traces)
    /* already initialized */
    return;

  traces = malloc(sizeof(struct trace_t) * __nb_traces);
}

void eztrace_initialize_gtg()
{
  /* the thread is waiting for something (sem_P, mutex_lock, etc.) */
  addContType("CT_Program", NULL, "Program");
  addContType("CT_Process", "CT_Program", "Process");
  addContType("CT_Thread", "CT_Process", "Thread");

  addStateType("ST_Program", "CT_Program", "Program state");
  addStateType("ST_Process", "CT_Process", "Process state");
  addStateType("ST_Thread", "CT_Thread", "Thread state");

  /* the thread is blocked */
  addEntityValue("STV_FLUSH", "ST_Thread", "EZTrace Flush", GTG_WHITE);

  /* the thread is blocked */
  addEntityValue("STV_Blocked", "ST_Thread", "Blocked", GTG_RED);
  /* the thread is computing */
  addEntityValue("STV_Working", "ST_Thread", "Working", GTG_BLUE);
  /* the thread is within a critical section (mutex, spinlock, ...) */
  addEntityValue("STV_Critical", "ST_Thread", "Critical Section", GTG_GREEN);

  addEntityValue("STV_EZTRACE_SYNC", "ST_Thread", "EZTrace synchronization", GTG_WHITE);

  addEventType("E_SigSegv", "CT_Thread", "SIGNAL Received");

  addContainer (0.00000, "C_Prog", "CT_Program", NULL, "Program", "0");
}


struct eztrace_event_handler* get_handler_info()
{
  return &__handler_info;
}

/* call printf if verbose mode is turned on */
#define DPRINTF(...)				\
  {						\
    if(verbose)					\
      printf(__VA_ARGS__);			\
  }

/* set to 1 if the current trace should be skipped next time.
 * It is mostly usefull when a trace starts with a delay (because
 * of MPI_Comm_spawn for example)
 */
int skip;

int* get_verbose()
{
  return &verbose;
}

int* get_nb_traces()
{
  return &nb_traces;
}

int* get_nb_start()
{
  return &nb_start;
}

struct trace_t* get_traces(int index)
{
  return &traces[index];
}

struct trace_t* get_cur_trace()
{
  return cur_trace;
}

void set_cur_trace(struct trace_t* p_trace)
{
  cur_trace = p_trace;
}

struct fxt_ev_64 *get_cur_ev()
{
  return cur_ev;
}

void set_cur_ev(struct fxt_ev_64 * p_ev)
{
  cur_ev = p_ev;
}

int* get_skip()
{
  return &skip;
}

/* Find the next event to be handled in a trace
 * Return 0 if there is no more event in this trace, or 1 otherwise
 */
int next_ev(int cur_trace_num)
{
  int ret = fxt_next_ev (traces[cur_trace_num].block, FXT_EV_TYPE_64,
			 (struct fxt_ev *) &traces[cur_trace_num].ev);
  traces[cur_trace_num].line_number++;
  if (ret != FXT_EV_OK)
    {
      traces[cur_trace_num].done = 1;
      fprintf (stderr, "no more block for trace #%d\n", cur_trace_num);
      return 0;
    }
  return 1;
}

/* Find the next trace event to handle
 * return the trace id that correspond to the trace to be handled
 */
static int __get_cur_ev() {
  int i;
  uint64_t min_time = -1;
  int min_trace = -1;

  /* First, make all the non-started traces progress */
  if(NB_START < NB_TRACES)
    for(i=0;i<NB_TRACES;i++)
      if(! get_traces(i)->start) {
        *get_skip() = 0;
	return i;
      }

  for(i=0; i<NB_TRACES; i++) {
    /* Only considere traces that
     * - have events to be handled
     * - do not wait for an external event
     */
    if((!get_traces(i)->done)
       && (!get_traces(i)->skip)
       && (CUR_TIME(i) < min_time)) {
      min_time = CUR_TIME(i);
      min_trace = i;
    }
    /* reset the skip value so that the trace is not skipped forever */
    if(!SKIP)
      get_traces(i)->skip = 0;
  }
  SKIP = 0;
  return min_trace;
}


/* initialize a container.
 * if new_container is NULL, a container is malloc'd
 * Return the address of the new container
 */
static struct eztrace_container_t* eztrace_create_container(struct eztrace_container_t* new_container, struct eztrace_container_t* parent_container)
{
  if(!new_container)
    new_container = malloc(sizeof(struct eztrace_container_t));
  new_container->parent = parent_container;
  new_container->nb_children = 0;
  new_container->children = NULL;
  new_container->p_trace = NULL;

  if(parent_container) {
    new_container->p_trace = parent_container->p_trace;
    parent_container->nb_children++;
    parent_container->children = realloc(parent_container->children,
					 sizeof(struct eztrace_container_t*) * parent_container->nb_children);
    parent_container->children[parent_container->nb_children-1] = new_container;
  }
  counters_new_container(new_container);
  hierarchical_array_new_container(new_container);

  return new_container;
}

void eztrace_create_containers(int trace_index) {
  /* initialize the root container */
  struct eztrace_container_t *p_container = &get_traces(trace_index)->root_container;
  eztrace_create_container(p_container, NULL);

  p_container->container_type = process;
  p_container->p_trace = get_traces(trace_index);

  /* create the process_info structure and initialize it */
  p_container->container_info = malloc(sizeof(struct process_info_t));
  struct process_info_t *p_process = (struct process_info_t *) p_container->container_info;
  p_process->pid = trace_index;
  p_process->container = p_container;

  ezt_hook_list_init(&(p_process->hooks));
}

void eztrace_create_ids(int trace_index) {
  struct eztrace_container_t *p_container = &get_traces(trace_index)->root_container;

  /* create the id and name strings */
  CREATE_PROCESS_ID_STR(p_container->id, p_container->p_trace->trace_id);
  CREATE_PROCESS_NAME_STR(p_container->name, trace_index);
}

/* create the process corresponding to a trace */
void add_process(int trace_id) {
  eztrace_create_containers(trace_id);
  eztrace_create_ids(trace_id);
}

/* add a thread to the process_info_t structure */
void add_pthread(int tid)
{
  struct eztrace_container_t *thread_container = eztrace_create_container(NULL, &CUR_TRACE->root_container);
  struct process_info_t *p_process = (struct process_info_t *) &CUR_TRACE->root_container.container_info;
  struct thread_info_t *p_thread = (struct thread_info_t*) malloc(sizeof(struct thread_info_t));

  thread_container->container_type = thread;
  thread_container->container_info = p_thread;

  CREATE_THREAD_ID_STR(thread_container->id, CUR_ID, tid);
  CREATE_THREAD_NAME_STR(thread_container->name, CUR_ID, tid);

  p_thread->tid = tid;
  p_thread->to_be_killed = 0;
  p_thread->processing_thread = NULL;
  p_thread->container = thread_container;
  p_thread->expected_code = 0;

  ezt_hook_list_init(&p_thread->hooks);

  sem_init(&p_thread->to_process, 0, 0);
}


static void __add_pthread()
{
  add_pthread(CUR_THREAD_ID);
}

/* PThread creation/destruction processing */
void new_thread (int tid)
{
  if(CUR_ID && (NB_TRACES == 1 || CUR_TRACE->start))
    {
      FUNC_NAME;

      add_pthread(tid);
      DECLARE_PROCESS_ID_STR(process_id, CUR_INDEX);
      DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

      if(__cur_mode == EZTRACE_CONVERT) {
	addContainer (CURRENT, thread_id, "CT_Thread", process_id, thread_id, "0");
	setState(CURRENT, "ST_Thread", thread_id, "STV_Working");
      }
    }
}

void
handle_new_thread ()
{
  new_thread(CUR_THREAD_ID);
}

void
handle_end_thread (void)
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  if(__cur_mode == EZTRACE_CONVERT && thread_id) {

    destroyContainer(CURRENT, thread_id, "CT_Thread");
  }
}

void
handle_thread_create (void)
{
  FUNC_NAME;
  if(NB_TRACES == 1 || CUR_TRACE->start) {
    add_pthread(CUR_THREAD_ID);
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    DECLARE_PROCESS_ID_STR(process_id, CUR_INDEX);
    if(__cur_mode == EZTRACE_CONVERT) {
      addContainer (CURRENT, thread_id, "CT_Thread", process_id, thread_id, "0");
    }
  }
}

void
handle_start_thread_join (void)
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_Blocked");
}

void
handle_stop_thread_join (void)
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}


void
handle_signal()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int signo = GET_PARAM(CUR_EV, 1);
  int nb_calls = GET_PARAM(CUR_EV, 2);
  char* event_str = NULL;
  asprintf(&event_str, "Signal %d received. Backtrace:", signo);
  int i;
  for(i=0;i<nb_calls; i++){
    fxt_next_ev (CUR_TRACE->block, FXT_EV_TYPE_64, (struct fxt_ev *) &CUR_TRACE->ev);
    char *str = CUR_EV->raw;
    char* tmp=event_str;
    asprintf(&event_str, "%s<br/>[%d] %s", tmp, i, str);
    free(tmp);
  }
  CHANGE() addEvent(CURRENT, "E_SigSegv", thread_id, event_str);
  free(event_str);
}

uint64_t add_delay_to_trace(int trace_num, uint64_t old_time, uint64_t new_time, const char* thread_id)
{
    struct trace_t* t = get_traces(trace_num);
    uint64_t delay = new_time - old_time;

    t->delay += delay;

    CHANGE() pushState(old_time/1000000.0, "ST_Thread", thread_id, "STV_EZTRACE_SYNC");

    CHANGE() popState(new_time/1000000.0, "ST_Thread", thread_id);

    return delay;
}



static struct ezt_list_t module_list;

void __core_init(void) __attribute__ ((constructor));
void __core_init(void)
{
  static int module_initialized = 0;
  if(!module_initialized) {
    module_initialized = 1;
    ezt_list_new(&module_list);
  }
}

/* This function calls the init handler for all the registered modules */
int __init_modules()
{
  struct ezt_list_token_t *token;

  ezt_list_foreach(&module_list, token) {

    struct eztrace_convert_module *p_module = (struct eztrace_convert_module *)token->data;
    p_module->init();
  }
}

/* This function calls the handle callback for all the registered modules
 * It returns 1 if the ev was handled by one module or 0 otherwise
 */
int __handle_event(struct fxt_ev_64 *ev)
{
  int ret = 0;
  struct ezt_list_token_t *token;

  ezt_list_foreach(&module_list, token) {

    struct eztrace_convert_module *p_module = (struct eztrace_convert_module *)token->data;
    if(p_module->handle)
      ret = p_module->handle(ev);

    /* The module handled the event, let's return directly*/
    if(ret) {
      goto out;
    }
  }
 out:
  return ret;
}

static int nb_handled_events = 0;
/* This function calls the handle_stats callback for all the registered modules
 * It returns 1 if the ev was handled by one module or 0 otherwise
 */
int __handle_stats(struct fxt_ev_64 *ev)
{
  int ret = 0;
  struct ezt_list_token_t *token;

  ezt_list_foreach(&module_list, token) {

    struct eztrace_convert_module *p_module = (struct eztrace_convert_module *)token->data;
    if(p_module->handle_stats)
      ret = p_module->handle_stats(ev);

    /* The module handled the event, let's return directly */
    if(ret) {
      nb_handled_events++;
      goto out;
    }
  }
 out:
  return ret;
}

/* This function calls the handle_stats callback for all the registered modules
 * It returns 1 if the ev was handled by one module or 0 otherwise
 */
void __print_stats()
{
  struct ezt_list_token_t *token;

  ezt_list_foreach(&module_list, token) {
    struct eztrace_convert_module *p_module = (struct eztrace_convert_module *)token->data;
    if(p_module->print_stats)
      p_module->print_stats();
  }
  printf("%d events handled\n", nb_handled_events);
}

void eztrace_convert_list()
{
  struct ezt_list_token_t *token;

  ezt_list_foreach(&module_list, token) {
    struct eztrace_convert_module *p_module = (struct eztrace_convert_module *)token->data;
    printf("%d\t%s", p_module->module_prefix, p_module->name);
    printf("\t%s\n", p_module->description);
  }

}

/* set to 0 if the loading of module should be silent */
int module_verbose = 0;

void eztrace_convert_register_module(struct eztrace_convert_module *p_module)
{
  if(p_module->api_version != EZTRACE_API_VERSION)
    fprintf(stderr, "Warning: module %s uses API version %x, but current version is %x\n",
	    p_module->name, p_module->api_version, EZTRACE_API_VERSION);
  struct ezt_list_token_t *token;
  /* Let's check wether another module with the same prefix is already registered */
  ezt_list_foreach(&module_list, token) {
    struct eztrace_convert_module *p_mod = (struct eztrace_convert_module *)token->data;

    if(p_module->module_prefix == p_mod->module_prefix) {
      fprintf(stderr, "Trying to register a module that is already registered. Module prefix is %x\n", p_mod->module_prefix);
      return;
    }
  }

  ezt_list_add(&module_list, &p_module->token);
  if(module_verbose)
    printf("module %s loaded\n", p_module->name);
}

char* cur_module_name = NULL;
#define SO_STRING DYNLIB_EXT

/* return 1 if the filename matches "eztrace_convert_*.so" */
static int filter(const struct dirent *entry)
{
  const char* filename = entry->d_name;

  /* check wether the string starts with "eztrace-convert-" */
  if (strncmp(filename,
	      "libeztrace-convert-",
	      strlen("libeztrace-convert-")))
    /* the string doesn't start with "eztrace-convert-" */
    return 0;


  /* check wether the next chars correspond to the module name
   * that we are looking for
   */
  filename += strlen("libeztrace-convert-");
  if (cur_module_name) {
    if (strncmp(filename,
		cur_module_name,
		strlen(cur_module_name)))
      /* the string doesn't start with "eztrace-convert-" */
      return 0;
    filename += strlen(cur_module_name);
  } else {
    filename = entry->d_name + strlen(entry->d_name) - strlen(SO_STRING);
  }
  if (strncmp(filename,
	      SO_STRING,
	      strlen(SO_STRING)))
    /* the string doesn't end with ".so" */
    return 0;

  /* check wether there are remaining chars after .so */
  filename += strlen(SO_STRING);
  if(filename[0])
    return 0;
  return 1;
}


/* Load a module from a specific libdir
 * if module_name is NULL, this function loads all the available
 * modules.
 * return the number of modules loaded
 */
static int __load_module_from_libdir(const char*module_name, const char*libdir)
{
  int nb_loaded = 0;
  /* update the module name we're looking for so that the filter function
   * can work as expected.
   */
  cur_module_name = (char*)module_name;

  if(module_name)
    DPRINTF("Looking for module %s in directory %s\n", module_name, libdir)
    else
      DPRINTF("Looking for any module in directory %s\n", libdir)

	struct dirent **namelist;
  int n;
  /* Get the list of files that match the module name in the libdir directory */
  n = scandir(libdir, &namelist, filter, alphasort);
  if (n < 0)
    perror("scandir");
  else {
    while (n--) {
      /* Get the full name of the file (path/libname.so) */
      char* libname = NULL;
      asprintf(&libname, "%s/%s", libdir, namelist[n]->d_name);

      /* Open the lib. The constructor of this lib is called and should register
       * the module by calling eztrace_convert_register_module()
       */
      void* dlret = dlopen(libname, RTLD_NOW);
      if(!dlret) {
	fprintf(stderr, "%s\n", dlerror());
      }
      DPRINTF("\tloaded: %s\n", libname)
	nb_loaded ++;
      free(namelist[n]);
      if(module_name) {
	/* the module was loaded, return from the
	   function so that the same module is not loaded again */
	free(namelist);
	goto out;
      }
    }
    free(namelist);
  }
 out:
  return nb_loaded;
}

/* Search a module and load it.
 * if module_name is NULL, this function loads all the available
 * modules.
 * return the number of modules loaded
 */
static int __load_all_modules(const char*module_name)
{
  /* number of modules loaded */
  int nb_loaded = 0;
  char* save_ptr = NULL;
  char* cur_path = NULL;
  char* lib_path = NULL;
  /* First, let's try in the default lib_path */
  nb_loaded = __load_module_from_libdir(module_name, EZTRACE_LIB_DIR);

  if(module_name && nb_loaded) {
    /* the module was loaded, return from the
       function so that the same module is not loaded again */
    goto out;
  }

  /* Module not found, let's try the lib_path specified by the user */
  lib_path = getenv("EZTRACE_LIBRARY_PATH");
  if(!lib_path) {
    /* No lib_path specify, we can't find any more plugin */
    goto out;
  }

  /* Iterate over the lib_path specified.
   * lib_path are separated by ':'
   */
  save_ptr = lib_path;
  cur_path = strtok_r(lib_path, ":", &save_ptr);
  while(cur_path) {

    nb_loaded += __load_module_from_libdir(module_name, cur_path);

    if(module_name && nb_loaded) {
      /* the module was loaded, return from the
	 function so that the same module is not loaded again */
      goto out;
    }
    cur_path = strtok_r(NULL, ":", &save_ptr);
  }
 out:
  return nb_loaded;
}

void load_modules(int mod_verb)
{
  char* module_list = getenv("EZTRACE_TRACE");
  module_verbose = mod_verb;
  int nb_loaded = 0;
  char* save_ptr = NULL;
  char* module = NULL;

  if(!module_list) {
    /* no env declares, so let's load everything */
    nb_loaded = __load_all_modules(NULL);
    /* We have loaded the pthread_core module which is not a
     * 'real' module, so let's decrement nb_loaded
     */
    nb_loaded--;
    goto out;
  }

  DPRINTF("loading modules\n")

    /* EZTRACE_TRACE is declared.
     * it should contain modules separated by " ", such
     * as "mpi coreblas pthread"
     * Let's iterate over these modules and load them once at a time
     */
    save_ptr = module_list;
  module = strtok_r(module_list, " ", &save_ptr);
  while(module) {
    DPRINTF("loading module %s\n", module);
    int loaded = __load_all_modules(module);
    if(!loaded)
      fprintf(stderr, "Cannot find module '%s'\n", module);
    nb_loaded += loaded;
    module = strtok_r(NULL, " ", &save_ptr);
  }
 out:
  if(mod_verb)
    printf("%d modules loaded\n", nb_loaded);
  return;

}

/* Tell the main thread that the current event has been processed */
void set_job_completed()
{
  sem_post(&job_processed);
}

/* Wait for the next event to handle */
void wait_for_next_job(struct thread_info_t * thread_id)
{
  sem_wait(&thread_id->to_process);
}

void set_cur_mode(enum eztrace_mode mode)
{
  __cur_mode = mode;
}

eztrace_mode_t get_mode()
{
  return __cur_mode;
}

void handle_start_flush()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_FLUSH");
}

void handle_stop_flush()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}

int process_one_event(struct fxt_ev_64 *ev)
{
  /* we just woke up, the event to handle is CUR_EV */
  int ret = 0;
  if(__cur_mode == EZTRACE_CONVERT)
    ret = __handle_event(ev);
  else
    ret = __handle_stats(ev);

  /* Let's ignore FxT calibration events */
  if(!ret)

    switch(ev->code)
      {
      case 0x320:
      case 0xffff:
      case 0xfffc:
      case 0xfffb:
      case 0xfffa:
	ret = 1;
	break;

      case FUT_START_FLUSH_CODE:
	handle_start_flush();
	ret = 1;
	break;
      case FUT_STOP_FLUSH_CODE:
	handle_stop_flush();
	ret = 1;
	break;

	/* Handle the pthread_core related codes */
      case FUT_NEW_THREAD:
	/* already processed by the main thread, we can ignore this event */
	ret = 1;
	break;
      case FUT_END_THREAD:
	handle_end_thread ();
	ret = 1;
	/* this thread won't be used anymore */
	return -1;
	break;
      case FUT_START_THREAD_JOIN:
	handle_start_thread_join ();
	ret = 1;
	break;
      case FUT_STOP_THREAD_JOIN:
	handle_stop_thread_join ();
	ret = 1;
	break;
      case FUT_SIGNAL_RECEIVED:
	handle_signal();
	ret = 1;
	break;

      default:
	ret = 0;
      }

  if(!ret && NB_START >= NB_TRACES) {
    /* This event is unknown :-( */
    fprintf (stderr, "[%d] unknown event.. %x at time %llu\n",
	     CUR_INDEX, (unsigned) ev->code, (long long unsigned) ev->time);
  }
  return ret;
}

/* Handle events issued by an application thread */
void* handle_event_thread(void* arg)
{
  struct thread_info_t *my_thread = (struct thread_info_t *) arg;

  while(1) {
    /* wait for an event to process */
    wait_for_next_job(my_thread);

    /* we just woke up, the event to handle is CUR_EV */
    int ret = process_one_event(CUR_EV);
    if(ret < 0)
      goto out;

    /* the event was processed, let's signal the main thread */
    set_job_completed();

  }
 out:
  sem_post(&job_processed);
  return NULL;
}

static int __handle_event_generic(struct fxt_ev_64 *ev)
{
  /* initialize stuff */
  static int first_time = 1;
  if(first_time) {
    first_time = 0;
    sem_init(&job_processed, 0, 0);
  }

  DECLARE_CUR_THREAD(cur_thread);
  if(cur_thread) {
    /* check wether another is waiting for this event */
    struct expected_code_t* exp = cur_thread->expected_code;
    while(exp) {
      if(exp->code == ev->code) {
	/* a thread is waiting for this event */
	/* wake up this thread */
	sem_post(&exp->semaphore);
	/* wait until the thread has processed the event */
	wait_for_main_thread();

	return 1;
      } else {
	exp = exp->next;
      }
    }
  }

  /* we just woke up, the event to handle is CUR_EV */
  int ret = process_one_event(ev);
  ret = ((ret<=0)?0:1);

  if(cur_thread && cur_thread->to_be_killed) {
    /* the current thread is marked "to be killed"
     * This means that for processing the current event, we had to block
     * and create a new thread for handling the following events. Now that
     * the event is processed, there are two handling threads, so let's
     * wake up the other thread and destroy the current thread.
     */
    cur_thread->to_be_killed = 0;
    wake_up_handler_thread();
    pthread_exit(NULL);
  }

  return ret;
}

int handle_event(struct fxt_ev_64 *ev)
{
  set_cur_mode(EZTRACE_CONVERT);
  return __handle_event_generic(ev);
}

int stats_handle_events(struct fxt_ev_64 *ev)
{
  set_cur_mode(EZTRACE_STATS);
  return __handle_event_generic(ev);
}


/* ask the event scheduler to call handle(data) next time trace #trace_index is scheduled */
void ask_for_replay(int trace_index,
		    void (*handle)(void*),
		    void* data);

struct __ezt_replay{
  void (*handle)(void*);
  void* data;
};

struct __ezt_replay *replays = NULL;

/* ask the event scheduler to call handle(data) next time trace #trace_index is scheduled
 * see handle_mpi_stop_waitall in src/modules/mpi/eztrace_convert_mpi.c for an example
 */
void ask_for_replay(int trace_index,
		    void (*handle)(void*),
		    void* data)
{
  if(!replays) {
    /* first time this function is called. allocate replays */
    int nb_traces = *(get_nb_traces());
    replays = malloc(sizeof(struct __ezt_replay) * nb_traces);
    int i;
    for(i=0; i< nb_traces; i++){
      replays[i].handle = NULL;
      replays[i].data = NULL;
    }
  }

  assert(replays[trace_index].handle == NULL);
  replays[trace_index].handle = handle;
  replays[trace_index].data = data;
}

/* execute a replay */
static void __execute_replay(struct __ezt_replay *r)
{
  void (*handle)(void*) = r->handle;
  void* data = r->data;

  /* remove the replay so that we don't loop infinitely */
  r->handle = NULL;
  r->data = NULL;

  /* execute the function */
  handle(data);
}

void* handle_one_event(void* arg)
{
  /* get the next event to process */
  __handler_info.cur_trace_nb = __get_cur_ev();
  set_cur_trace(get_traces(__handler_info.cur_trace_nb));
  set_cur_ev(&get_cur_trace()->ev);

  int ret = 0;
  if(replays && replays[__handler_info.cur_trace_nb].handle) {
    /* execute the replay */

    __execute_replay(&replays[__handler_info.cur_trace_nb]);

  } else {

    
    ret = (__cur_mode == EZTRACE_CONVERT)? handle_event(CUR_EV) : stats_handle_events(CUR_EV);
    __handler_info.nb_handled += ret;

  }

  /* If current event was skipped, do not step forward the trace or this event will be 'lost' */
  if( !SKIP && !next_ev(__handler_info.cur_trace_nb))
    __handler_info.nb_done++;

  return NULL;
}

void* handle_all_events(void* arg)
{
  wait_for_main_thread();

  while (__handler_info.nb_done < NB_TRACES)
    handle_one_event(arg);

  sem_post(&__handler_info.events_processed);
}

void wake_up_handler_thread()
{
  sem_post(&main_thread_sem);
}

void wait_for_an_event(int trace_index, uint64_t code)
{
  if(!next_ev(trace_index)) {
    fprintf(stderr, "Warning: trace %d ends, but I am waiting for an event !\n", trace_index);
    return;
  }

  if(CUR_TRACE->ev.code == code)
    return;

  /* The current thread expects a specific event, but another code appeared.
   * (This usually happens due to a race condition between thread when instrumenting)
   */

  /* create another thread for processing the unneeded events */
  new_handler_thread();

  DECLARE_THREAD_INFO(p_thread, CUR_INDEX, CUR_THREAD_ID);
  /* add the {code,semaphore} to the list of expected events */
  struct expected_code_t* exp_code = malloc(sizeof(struct expected_code_t));
  exp_code->code = code;
  sem_init(&exp_code->semaphore, 0, 0);
  exp_code->next = p_thread->expected_code;

  /* wake up the thread we just created */
  wake_up_handler_thread();

  /* wait until the processing thread finds our events */
  sem_wait(&exp_code->semaphore);

  /* make sure the event code is the right one */
  assert(CUR_TRACE->ev.code == code);

  /* remove the {code,semaphore} from the list of expected events */
  struct expected_code_t* cur_code, *next_code;
  cur_code = p_thread->expected_code;
  next_code = cur_code->next;
  while(next_code != exp_code) {
    cur_code = next_code;
    next_code = next_code->next;
  }
  cur_code->next = next_code->next;
  free(exp_code);

  /* Now that the event is processed, there are two handling threads, we need to destroy
   * the current thread. We can't do this right now (because we need to process the event
   * we were expecting). So, mark the thread "to be killed" so that someone destroys it
   * later (this is done in __handle_event_generic)
   */
  p_thread->to_be_killed = 1;
}

void wait_for_main_thread()
{
  sem_wait(&main_thread_sem);
}


void new_handler_thread()
{
  DECLARE_CUR_THREAD(p_thread);

  pthread_t *p_new_thread = malloc(sizeof(pthread_t));
  pthread_create(p_new_thread, NULL, handle_all_events, NULL);

  p_thread->processing_thread = main_thread;
  main_thread = p_new_thread;
}

void create_main_thread()
{
  main_thread = malloc(sizeof(pthread_t));
  pthread_create(main_thread, NULL, handle_all_events, NULL);
}

/* destroy a container and its children */
static void __finalize_container(struct eztrace_container_t* p_cont)
{
  if(!p_cont)
    return;

  int i;
  /* finalize the children */
  for(i=0; i<p_cont->nb_children; i++) {
    __finalize_container(p_cont->children[i]);
    free(p_cont->children[i]);
  }

  /* finalize the container */
  if(p_cont->container_type == thread) {
    struct thread_info_t *p_thread = (struct thread_info_t*) p_cont->container_info;
    /* free the pthread pointer */
    if(p_thread->processing_thread) {
      free(p_thread->processing_thread);
    }

    /* free the thread hooks */
    ezt_hook_list_free(&p_thread->hooks);
  } else {
    /* the container is a process */
    struct process_info_t *p_process = (struct process_info_t*) p_cont->container_info;
    ezt_hook_list_free(&p_process->hooks);
  }

  free(p_cont->container_info);
}

void eztrace_convert_finalize()
{
  int i;
  /* free all the containers */
  for(i=0; i<NB_TRACES; i++) {
    struct trace_t* cur_trace = get_traces(i);
    struct eztrace_container_t* cur_cont = &(cur_trace->root_container);
    __finalize_container(cur_cont);
  }
  free(main_thread);
}
