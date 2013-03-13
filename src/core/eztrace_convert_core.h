/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_CONVERT_CORE_H
#define EZTRACE_CONVERT_CORE_H
#include "eztrace_convert_types.h"


eztrace_mode_t get_mode();

/* return a pointer to the verbose variable */
int* get_verbose();
/* return a pointer to the nb_traces variable */
int* get_nb_traces();
/* return a pointer to the nb_start variable */
int* get_nb_start();

/* return a pointer to trace #index */
struct trace_t* get_traces(int index);
/* return a pointer to the trace whose event is being considered */
struct trace_t* get_cur_trace();
/* change the trace whose event is being considered */
void set_cur_trace(struct trace_t* p_trace);

/* return a pointer to the current event */
struct fxt_ev_64 *get_cur_ev();
/* change the current event */
void set_cur_ev(struct fxt_ev_64 * p_ev);

/* The current event has not been handled, because requires another event (used in MPI_Comm_spawn) */
int* get_skip();

/* PThread creation/destruction processing */
void new_thread (int tid);

/* add a thread to the process_info_t structure */
void add_pthread(int tid);

/* initialize the container structures. */
void eztrace_create_containers(int trace_index);

/* initialize the id and name fields of the process container structures */
void eztrace_create_ids(int trace_index);



/* Initialize eztrace_convert_core */
void eztrace_convert_init(unsigned nb_traces);
/* Initialize GTG (define the default States used in EZTrace)  */
void eztrace_initialize_gtg();
/* Finalize eztrace_convert_core */
void eztrace_convert_finalize();


/* load available modules */
void load_modules(int verbose);
/* print the list of currently loaded modules */
void eztrace_convert_list();

int __init_modules();
int __handle_event(struct fxt_ev_64 *ev);
int __handle_stats(struct fxt_ev_64 *ev);
void __print_stats();

void handle_new_thread ();
void handle_end_thread ();
void handle_thread_create ();
void handle_start_thread_join ();
void handle_stop_thread_join ();

void* handle_one_event(void* arg);
int handle_event(struct fxt_ev_64 *ev);

void set_job_completed();
void wait_for_next_job(struct thread_info_t * thread_id);

/* add a delay to trace trace_num
 * @param trace_num index of the trace to modify
 * @param old_time original time in the trace
 * @param new_time new time in the trace
 * @param thread_id identifier of the thread that detects the delay
 */
uint64_t add_delay_to_trace(int trace_num, uint64_t old_time, uint64_t new_time, const char* thread_id);


/* In order to avoid some deadlocks that may occur when an event handler handles
 * several events in a raw (because it records too many parameters to fit in one
 * call to EZTRACE_EVENT), eztrace_convert is multithreaded.
 *
 * In most cases, only one thread is run. However, when a handler needs to wait for
 * a particular event, it creates a new thread that becomes the main thread. The
 * previous main thread then blocks on a semaphore until the appropriate event is found.
 * It can then handle the event. This thread should then call pthread_exit().
 */


/* 'duplicate' the main thread. The new main_thread run handle_events.
 * The 'old' main_thread (ie. the current thread) then has to wait until the
 * main thread wakes it up.
 */
void new_handler_thread();

/* Wake up an handler thread */
void wake_up_handler_thread();

/* Wait until the main thread notifies that an event has to be processed
 * for a particular thread.
 */
void wait_for_an_event(int trace_index, uint64_t code);

void* handle_all_events(void* arg);

struct eztrace_event_handler* get_handler_info();


/* Ask the event scheduler to call handle(data) next time trace #trace_index is scheduled
 * This can be used when the processing of an event has started but has to wait for an
 * event to happen before resuming the processing.
 * see handle_mpi_stop_waitall in src/modules/mpi/eztrace_convert_mpi.c for an example
 */
void ask_for_replay(int trace_index,
		    void (*handle)(void*),
		    void* data);


#endif	/* EZTRACE_CONVERT_CORE_H */
