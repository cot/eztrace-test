/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_CONVERT_H
#define EZTRACE_CONVERT_H

#define _GNU_SOURCE

#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#include "eztrace_convert_types.h"
#include "eztrace_convert_core.h"
#include "eztrace_convert_macros.h"


/* register a plugin to eztrace_convert */
void eztrace_convert_register_module(struct eztrace_convert_module *p_module);

/* Find the next event to be handled in a trace
 * Return 0 if there is no more event in this trace, or 1 otherwise
 */
int next_ev(int cur_trace_num);


/* add the current thread to the output trace and initialize
 * all the corresponding structures.
 */
void new_thread (int tid);

/* retrieve a container that corresponds to a thread id */
static inline
struct eztrace_container_t * get_thread_cont_from_id(int trace_id, int thread_id)
{
  struct eztrace_container_t * cont = (&(get_traces(trace_id)->root_container));
  unsigned i;
  for(i=0; i<cont->nb_children; i++) {
    struct eztrace_container_t * child_cont = cont->children[i];
    struct thread_info_t *ret = (struct thread_info_t *)child_cont->container_info;
    if(ret->tid == thread_id)
      return child_cont;
  }

  return NULL;
}

/* if the verbose option is set (when eztrace_convert is invoked)
 * this prints the name of the current function
 */
static inline void __func_name(int id, float time, const char* function)
{
  if(VERBOSE)
    fprintf(stderr, "[%d] [%f] \t%s\n", id, time,  function);
}

#endif	/* EZTRACE_CONVERT_H */
