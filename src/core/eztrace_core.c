/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

/* -*- c-file-style: "GNU" -*- */
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <execinfo.h>
#include "ev_codes.h"
#include "eztrace.h"

/* -1 means not initialized
 * 0 means don't record counters
 * >0 means record counters
 */
int record_hardware_counter = -1;


int eztrace_debug_mode = 0;

static pthread_key_t protect_on;

static int PROF_BUFFER_SIZE = (16*1024*1024);

static char PROF_FILE_USER[1024];

static int prof_activated = 0;
fxt_t fut;

/* number of init functions to be called when the eztrace_start
 * is called.
 */
int nb_module = 0;

static void init_recursion_shield()
{
  static int init_done = 0;
  if(!init_done) {
    pthread_key_create(&protect_on, NULL);
    init_done = 1;
  }
}

void set_launcher_env() {
   setenv("TESTLAUNCHER", "1", 1);
}

void unset_launcher_env() {
   setenv("TESTLAUNCHER", "0", 1);
}

int is_in_launcher() {
   if (getenv("TESTLAUNCHER")!=NULL) {
      if (strcmp(getenv("TESTLAUNCHER"), "1")==0) 
         return 1;
   }
   return 0;
}

#ifndef EZTRACE_AUTOSTART

/* array of init functions */
eztrace_function_t __init_routines[16];

void eztrace_register_init_routine(eztrace_function_t init_func)
{
  __init_routines[nb_module++] = init_func;
}

#endif	/* EZTRACE_AUTOSTART */

static void __eztrace_set_buffer_size()
{
  char* res= getenv("EZTRACE_BUFFER_SIZE");
  if(res) {
    PROF_BUFFER_SIZE = atoi(res);
  }
}

static char* __eztrace_get_filedir()
{
  char* res= getenv("EZTRACE_TRACE_DIR");
  int ret;
  if(!res) {
    ret = asprintf(&res, "/tmp");
  }
  return res;
}

void eztrace_set_filename(char* name)
{
  sprintf(PROF_FILE_USER, "%s/%s_%s",__eztrace_get_filedir(), getenv("USER"), name);
  fut_set_filename(PROF_FILE_USER);
}

/* return the library name from a symbol string */
static char* get_lib_name(char*symbol)
{
  char* ret;
  int begin = 0;
  int end = -1;
  int i = 0;

  /* the format of symbol is : '/path/to/libxxx.so (function+0xoffset) [0xaddress]*/
  /* so we need to locate the last / and the first ( */
  while(symbol[i] != 0){
    if(symbol[i] == '/')
      begin = i+1;
    if(symbol[i] == '(') {
      end = i;
      break;
    }
    i++;
  }

  ret = &symbol[begin];
  /* replace ( with a \0 */
  if(end>=0)
    symbol[end] = 0;
  return ret;

}

/* return the function name (or library name if the function name is unknown)
 * from a symbol string.
 * This function may alter the symbol string.
 */
static char* get_function_name(char* symbol)
{
  char* ret = symbol;
  int len = strlen(symbol);
  int begin;
  int end;
  int i;

  /* the format of symbol is : 'libxxx.so (function+0xoffset) [0xaddress]*/
  /* the goal is to retrieve the function+0xoffset string */
  for(i=0; i<len; i++) {
    if(symbol[i] == '(') {
      begin = i;
      if(symbol[i+1] == '+' || symbol[i+1] == ')') {
	return get_lib_name(symbol);
      }
    }
    if(symbol[i] == ')') {
      end = i;
      break;
    }
  }

  ret = &symbol[begin+1];
  symbol[end] = 0;
  return ret;
}

void eztrace_error_handler(int signo)
{
  static volatile int shield = 0;
  /* in case several thread receive signals simultaneously, only the first one
   * handle it.
   */
  while(shield);
  shield = 1;

  set_recursion_shield_on();

  EZT_PRINTF(0, "[EZTrace] signal %d catched.\n", signo);
  void* buffer[50];

  /* get pointers to functions */
  int nb_calls = backtrace(buffer, 50);
  char **functions;

  functions = backtrace_symbols(buffer, nb_calls);
  int i;
  EZTRACE_EVENT2(FUT_SIGNAL_RECEIVED, signo, nb_calls);
  for(i=0;i<nb_calls; i++) {
    char* ptr = get_function_name(functions[i]);
    FUT_DO_PROBESTR(FUT_CALLING_FUNCTION, ptr);
  }
  free(functions);

  set_recursion_shield_off();
}

void eztrace_signal_handler(int signo)
{
  static volatile int shield = 0;
  /* in case several thread receive signals simultaneously, only the first one
   * handle it.
   */
  while(shield);
  shield = 1;

  EZT_PRINTF(0, "EZTrace received signal %d...\n", signo);
  if(signo == SIGSEGV)
    eztrace_error_handler(signo);

  eztrace_stop();
  EZT_PRINTF(0, "Signal handling done\n");
  exit(EXIT_FAILURE);
  //signal(signo, SIG_DFL);
}

static void __eztrace_set_sighandler() {
  char* res = getenv("EZTRACE_NO_SIGNAL_HANDLER");

  if(!res || !strncmp(res, "0", 2)) {
    signal(SIGSEGV, eztrace_signal_handler);
    signal(SIGINT, eztrace_signal_handler);
    signal(SIGTERM, eztrace_signal_handler);
    signal(SIGABRT, eztrace_signal_handler);
    signal(SIGHUP, eztrace_signal_handler);

  }
}

void
eztrace_start ()
{
  /* avoid executing this function several times */
  if (prof_activated)
    return;

  unsigned threadid;
  prof_activated = 1;

  char* debug_mode= getenv("EZTRACE_DEBUG");
  if(debug_mode) {
    eztrace_debug_mode = atoi(debug_mode);
    EZT_PRINTF(0, "EZTrace Debug mode enabled (trace level: %d)\n", eztrace_debug_mode);
  }

  EZT_PRINTF (0, "Starting EZTrace...\n ");
  eztrace_set_filename ("eztrace_log_rank_1");
  threadid = CUR_TID;

  __eztrace_set_buffer_size();

  /* make sure eztrace_stop is called when the program stops */
  atexit (eztrace_stop);
  __eztrace_set_sighandler();


  char* allow_flush= getenv("EZTRACE_FLUSH");
  if(allow_flush && strncmp(allow_flush, "0", 2)) {
    EZT_PRINTF(0, "EZTrace Flush enabled\n");
    enable_fut_flush();
  }

  fut_enable_tid_logging();
  /* start FxT */
  if (fut_setup (PROF_BUFFER_SIZE, FUT_KEYMASKALL, threadid) < 0)
    {
      perror ("fut_setup");
      exit (EXIT_FAILURE);
    }

  init_recursion_shield();

  /* the current thread needs to be registered since eztrace won't
   * intercept any pthread_create for this one
   */
  EZTRACE_EVENT0 (FUT_NEW_THREAD);

  if(record_hardware_counter <0)
    record_hardware_counter = 0;

#ifndef EZTRACE_AUTOSTART
  /* call the initialisation routines that were registered */
  int i;
  for(i=0; i<nb_module; i++)
	  __init_routines[i]();
#endif	/* EZTRACE_AUTOSTART */
  EZT_PRINTF (0, "done\n");

}

void
eztrace_stop ()
{
  if (!prof_activated)
    return;
  EZTRACE_EVENT0 (FUT_END_THREAD);
  prof_activated = 0;
  fut_endup (PROF_FILE_USER);
  fut_done ();
  EZT_PRINTF (0, "Stopping EZTrace... saving trace  %s\n", PROF_FILE_USER);
}


void eztrace_generic(uint32_t code, int nbargs, ...)
{
  int i;
  va_list args;
  uint64_t arg_array[5];

  va_start(args, nbargs);
  for(i=0; i<nbargs; i++)
    arg_array[i] = va_arg(args, uint64_t);

  switch(nbargs) {
  case 0: EZTRACE_EVENT0(code); break;
  case 1: EZTRACE_EVENT1(code, arg_array[0]); break;
  case 2: EZTRACE_EVENT2(code, arg_array[0], arg_array[1]); break;
  case 3: EZTRACE_EVENT3(code, arg_array[0], arg_array[1], arg_array[2]); break;
  case 4: EZTRACE_EVENT4(code, arg_array[0], arg_array[1], arg_array[2], arg_array[3]); break;
  }
}

void
eztrace_code0(uint32_t code)
{
  EZTRACE_EVENT0(code);
}

void eztrace_code1(uint32_t code, uint64_t arg1)
{
  EZTRACE_EVENT1(code, arg1);
}

void eztrace_code2(uint32_t code, uint64_t arg1, uint64_t arg2)
{
  EZTRACE_EVENT2(code, arg1, arg2);
}

void eztrace_code3(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
  EZTRACE_EVENT3(code, arg1, arg2, arg3);
}

void eztrace_code4(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
  EZTRACE_EVENT4(code, arg1, arg2, arg3, arg4);
}

void eztrace_code5(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
  EZTRACE_EVENT5(code, arg1, arg2, arg3, arg4, arg5);
}

int recursion_shield_on()
{
  init_recursion_shield();
  void* ret=NULL;
  ret = pthread_getspecific(protect_on);
  return (ret != NULL);
}

void set_recursion_shield_on()
{
  init_recursion_shield();
  pthread_setspecific (protect_on, (void*)1);
}
void set_recursion_shield_off()
{
  pthread_setspecific (protect_on, (void*)NULL);
}

#ifdef RECORD_BACKTRACES
/* record an event (code=FUT_CALLING_FUNCTION) with the calling function name */
void record_backtrace()
{
  set_recursion_shield_on();

  /* the current backtrace looks like this:
   * 0 - record_backtrace()
   * 1 - eztrace_callback()
   * 2 - calling_function
   *
   * So, we need to get the name of the function in frame 2.
   */
  void* buffer[3];
  /* get pointers to functions */

#if 1
  int nb_calls = backtrace(buffer, 3);
  assert(nb_calls>=3);

  char **functions;
  functions = backtrace_symbols(buffer, nb_calls);
  FUT_DO_PROBESTR(FUT_CALLING_FUNCTION, functions[2]);
  free(functions);
#else
  /* todo: it may be interesting (for performance reasons) to
   * use __builtin_frame_address or __builtin_return_address.
   */
  //  void *caller =  __builtin_frame_address (1);
  void* caller[3];
  caller[0] = NULL;
  caller[1] = __builtin_return_address (0);
  caller[2] = __builtin_return_address (1);
  char **functions = backtrace_symbols(&caller, 3);
  FUT_DO_PROBESTR(FUT_CALLING_FUNCTION, functions[2]);
  free(functions);
#endif

  set_recursion_shield_off();
}

#else
void record_backtrace() { }
#endif

