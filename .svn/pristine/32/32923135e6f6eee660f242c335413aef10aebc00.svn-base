/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

/* This file contains the methods invoked during the conversion of
 * the trace (ie. during eztrace_convert).
 * After compilation, this file is transformed into the plugin
 * libeztrace_convert_example.so
 * When this plugin is loaded by eztrace_convert, its constructor
 * (libinit() ) should register the plugin and specify various callbacks
 */

#include <stdio.h>
#include <GTG.h>
#include "example_ev_codes.h"
#include "eztrace_convert.h"

int eztrace_convert_example_init();
int handle_example_events(struct fxt_ev_64 *ev);

void handle_example_function1_entry();
void handle_example_function1_exit();

void handle_example_function2_entry();
void handle_example_function2_exit();

void print_example_stats();
int handle_example_stats(struct fxt_ev_64 *ev);


/* Constructor of the plugin.
 * This function registers the current module to eztrace_convert
 */
struct eztrace_convert_module example_module;
void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  /* Specify the initialization function.
   * This function will be called once all the plugins are loaded
   * and the trace is started.
   * This function usually declared StateTypes, LinkTypes, etc.
   */
  example_module.init = eztrace_convert_example_init;

  /* Specify the function to call for handling an event
   */
  example_module.handle = handle_example_events;

  /* Specify the function to call for handling an event when eztrace_stats is called
   */
  example_module.handle_stats = handle_example_stats;

  /* Print the results of statistics
   */
  example_module.print_stats = print_example_stats;

  /* Specify the module prefix */
  example_module.module_prefix = EXAMPLE_EVENTS_ID;

  asprintf(&example_module.name, "example");
  asprintf(&example_module.description, "Module for Example library");


  example_module.token.data = &example_module;

  /* Register the module to eztrace_convert */
  eztrace_convert_register_module(&example_module);

  printf("module Example loaded\n");
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
  printf("unloading module Example\n");
}



/*
 * This function will be called once all the plugins are loaded
 * and the trace is started.
 * This function usually declared StateTypes, LinkTypes, etc.
 */
int
eztrace_convert_example_init()
{
  addEntityValue("function1_alias", "ST_Thread", "Doing function1", GTG_PINK);
  addEntityValue("function2_alias", "ST_Thread", "Doing function2", GTG_BLACK);
  return 0;
}

/* declare a few variable for computing statistics */
static int nb_function1 = 0;
static int nb_function2 = 0;
static float total_time_in_function1 = 0;
static float total_time_in_function2 = 0;
static float last_start_time_function1 = 0;
static float last_start_time_function2 = 0;

/* Function called for handling an event when eztrace_stats is called
 * It shall return 1 if the event was handled successfully or
 * 0 otherwise.
 */
int
handle_example_stats(struct fxt_ev_64 *ev)
{
  /* depending on the event code, let's count the number of calls to function1 and function2
   * for both function, compute the average duration of the function call
   */
  switch(ev->code)
    {
    case FUT_EXAMPLE_FUNCTION1_ENTRY:
      nb_function1 ++;
      last_start_time_function1 = CURRENT;
      break;
    case FUT_EXAMPLE_FUNCTION1_EXIT:
      /* WARNING: this does not support support reentrant calls to function1.
       */
      total_time_in_function1 += CURRENT - last_start_time_function1;
      break;

    case FUT_EXAMPLE_FUNCTION2_ENTRY:
      nb_function2 ++;
      last_start_time_function2 = CURRENT;
      break;
    case FUT_EXAMPLE_FUNCTION2_EXIT:
      /* WARNING: this does not support support reentrant calls to function2.
       */
      total_time_in_function2 += CURRENT - last_start_time_function2;
      break;

    default:
      /* The event was not handled */
      return 0;
    }
  return 1;
}

/* Print the results of statistics.
 */
void print_example_stats() {
  printf ( "\nLibExample:\n");
  printf (   "-----------\n");

  if(nb_function1) {
    /* Only print this when calls to function1 were detected */
    printf ( "%d calls to Function1\n", nb_function1);
    printf ( "\taverage duration: %f us\n", total_time_in_function1/nb_function1);
  }

  if(nb_function2) {
    /* Only print this when calls to function2 were detected */
    printf ( "%d calls to Function2\n", nb_function2);
    printf ( "\taverage duration: %f us\n", total_time_in_function2/nb_function2);
  }
}

/* This function is called by eztrace_convert for each event to
 * handle.
 * It shall return 1 if the event was handled successfully or
 * 0 otherwise.
 */
int
handle_example_events(struct fxt_ev_64 *ev)
{
  switch(ev->code)
    {
    case FUT_EXAMPLE_FUNCTION1_ENTRY:
      handle_example_function1_entry();
      break;
    case FUT_EXAMPLE_FUNCTION1_EXIT:
      handle_example_function1_exit();
      break;

    case FUT_EXAMPLE_FUNCTION2_ENTRY:
      handle_example_function2_entry();
      break;
    case FUT_EXAMPLE_FUNCTION2_EXIT:
      handle_example_function2_exit();
      break;

    case FUT_STATIC_EXAMPLE_FUNCTION_ENTRY:
      fprintf(stderr, "static exemple function entry\n");
      break;
    case FUT_STATIC_EXAMPLE_FUNCTION_EXIT:
      fprintf(stderr, "static exemple function exit\n");
      break;

    default:
      /* The event was not handled */
      return 0;
    }
  return 1;
}


/* This function is called by handle_example_events when reaching
 * the FUT_EXAMPLE_FUNCTION1_ENTRY event.
 */
void
handle_example_function1_entry ()
{
  /* FUNC_NAME is used for debugging. If -v is passed to eztrace_convert,
   * FUNC_NAME prints the name of the function
   */
  FUNC_NAME;

  /* Initialize a char* thread_id and generate a string that
   * identifies the thread that issued the current event
   */
  INIT_THREAD_ID(thread_id);

  /* Update the state of the thread in the output trace. */
  CHANGE() pushState (CURRENT, "ST_Thread", thread_id, "function1_alias");

  free(thread_id);
}

/* This function is called by handle_example_events when reaching
 * the FUT_EXAMPLE_FUNCTION1_EXIT event.
 */
void
handle_example_function1_exit ()
{
  FUNC_NAME;
  INIT_THREAD_ID(thread_id);

  /* Restore the state of the thread to its previous state in the
   * output trace.
   */
  CHANGE() popState (CURRENT, "ST_Thread", thread_id);

  free(thread_id);
}




/* This function is called by handle_example_events when reaching
 * the FUT_EXAMPLE_FUNCTION2_ENTRY event.
 */
void
handle_example_function2_entry ()
{
  FUNC_NAME;
  INIT_THREAD_ID(thread_id);

  /* Update the state of the thread in the output trace. */
  CHANGE() pushState (CURRENT, "ST_Thread", thread_id, "function2_alias");

  free(thread_id);
}

/* This function is called by handle_example_events when reaching
 * the FUT_EXAMPLE_FUNCTION2_EXIT event.
 */
void
handle_example_function2_exit ()
{
  FUNC_NAME;
  INIT_THREAD_ID(thread_id);

  /* Restore the state of the thread to its previous state in the
   * output trace.
   */
  CHANGE() popState (CURRENT, "ST_Thread", thread_id);

  free(thread_id);
}
