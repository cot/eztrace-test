/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_CONVERT_TYPES_H
#define EZTRACE_CONVERT_TYPES_H
#include <semaphore.h>
#include <stdint.h>

#ifdef EXTERNAL_FXT
#include <fxt/fxt-tools.h>
#include <fxt/fut.h>
#else
#include <fxt-tools.h>
#include <fut.h>
#endif

#include "eztrace_hook.h"

struct eztrace_convert_module;
struct process_info_t;
struct thread_info_t;
struct trace_t;

/* application pointer. This pointer is invalid in the current 
 * process, but its value corresponds to an object in the
 * application.
 */
typedef void* app_ptr;

enum eztrace_mode {
  EZTRACE_STATS,
  EZTRACE_CONVERT,
};

typedef enum eztrace_mode eztrace_mode_t;

/* Describe a plugin */
struct eztrace_convert_module {
  uint32_t api_version;
  int (*init)();
  int (*handle)(struct fxt_ev_64* ev);
  int (*handle_stats)(struct fxt_ev_64* ev);
  void (*print_stats)();
  uint8_t module_prefix;
  char* name;
  char* description;
  struct ezt_list_token_t token;
};

enum container_type_t{
  process,
  thread,
};

/* contains generic information about a process or a thread */
struct eztrace_container_t {
  char* id;			/* string that identify the object */
  char* name;			/* string that describes the object */
  struct eztrace_container_t *parent; /* parent container */
  unsigned nb_children;		      /* number of children containers */
  struct eztrace_container_t **children; /* list of children containers */
  enum container_type_t container_type;
  void* container_info;		/* pointer to a process_info_t or thread_info_t */
  struct trace_t *p_trace;	/* pointer to the trace that describe this process */
  struct hierarchical_array* arrays;
  struct eztrace_counter* counters;
};


struct expected_code_t{
  uint64_t code;
  sem_t semaphore;
  struct expected_code_t* next;
};

/* Contains information about a thread.
 */
struct thread_info_t {
  int tid;			/* thread id */
  struct eztrace_container_t *container; /* container that corresponds to this thread */
  int to_be_killed;
  struct expected_code_t *expected_code;
  sem_t to_process;
  pthread_t *processing_thread;
  struct ezt_hook_list_t hooks;	/* list of hooks */
};

/* Contains information about a process.
 */
struct process_info_t{
  int pid;			/* process id */
  struct eztrace_container_t *container; /* container that corresponds to this process */
  struct ezt_hook_list_t hooks;
};

struct trace_t {
  uint64_t start_time; 		/* first timestamp of the trace */
  uint64_t delay;		/* if the trace started later than the other ones (ie. comm_spawn) */
  char *input_filename;
  char* trace_id;              /* Identifier of the trace in the output file */
  fxt_blockev_t block;
  struct fxt_ev_64 ev;
  int id; 			/* identifier in the traces array */
  int rank;			/* MPI rank. It may be different from the id */
  int start; 			/* set to 0 until the MPI_Init happen */
  int done;			/* No more event to handle */
  int skip;			/* Skip this trace. This is used when we have to wait for another trace to produce an event (mpi_comm_spawn for example) */
  int line_number;
  struct eztrace_container_t root_container; /* process corresponding to the trace */
};

struct eztrace_event_handler {
  int cur_trace_nb;
  int nb_done;
  int nb_handled;
  sem_t events_processed;
};

#endif	/* EZTRACE_CONVERT_TYPES_H */
