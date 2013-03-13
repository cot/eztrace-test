/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_STATS_CORE_H
#define EZTRACE_STATS_CORE_H
#include "eztrace_convert.h"

void eztrace_stats_set_output_dir(const char*path);
const char* eztrace_stats_get_output_dir();

/* This defines a hierarchical counter mechanism that permits to summarize
 * counters in a hierarchical way: a counter is maintained for each thread/process/whatever.
 * Example of output:
CT_Process #0 --        Total: 43812.000000     MPI: 20464.000000       OpenMP: 23348.000000
   P#0_T#3923150688 --  Total: 10832.000000     MPI: 5370.000000        OpenMP: 5462.000000
   P#0_T#3733358336 --  Total: 10321.000000     MPI: 4741.000000        OpenMP: 5580.000000
   P#0_T#3724965632 --  Total: 15568.000000     MPI: 7082.000000        OpenMP: 8486.000000
   P#0_T#3715507968 --  Total: 7091.000000      MPI: 3271.000000        OpenMP: 3820.000000
 */

/* Each counter contains a vector of double (in the example, the vector contains <MPI,OpenMP>)
 * for each eztrace_container.
 */
typedef uint32_t eztrace_counter_id;
typedef struct eztrace_container_t* p_eztrace_container;

/* Create a new counter whose id is counter_id. This counter contains vectors that consist in
 * vector_size items. The names of each of the vector item are stores in names
 */
void attach_counter(eztrace_counter_id counter_id, unsigned vector_size, char**names);

/* Allocate counters in a newly created container */
void counters_new_container(p_eztrace_container p_cont);

/* Return a pointer to the vector item #index from the counter #counter_id of a specific container */
double* get_counter(p_eztrace_container container, eztrace_counter_id counter_id, unsigned index);

/* Print the values of a specific counter */
void print_counters(eztrace_counter_id id);


/* Add /value/ to a counter for the current thread */
static inline
void counter_add(uint32_t counter_id, uint32_t module_id, double value)
{
  double* v = get_counter(GET_THREAD_CONTAINER(CUR_INDEX, CUR_THREAD_ID),
			  counter_id,
			  module_id);
  if (v) {
    (*v) += value;
  }
}



/* define a 'generic' counter that counts min/max/average */
#define STAT_COUNTER_T(type)			\
  struct stat_##type##_counter_t {		\
    type sum;					\
    type min;					\
    type max;					\
  }
#define stat_counter_t(type) struct stat_##type##_counter_t

/* define uint64_t counters */
STAT_COUNTER_T(uint64_t);
/* define double counters */
STAT_COUNTER_T(double);


/* initialize a double counter */
#define __init_double_stat_counter(counter)			\
  {								\
    ((struct stat_double_counter_t* ) counter)->sum = 0;	\
    ((struct stat_double_counter_t* ) counter)->min = DBL_MAX;	\
    ((struct stat_double_counter_t* ) counter)->max = DBL_MIN;	\
}

/* initialize a uint64_t counter */
#define  __init_uint64_t_stat_counter(counter)				\
  {									\
    ((struct stat_uint64_t_counter_t*)counter)->sum = 0;		\
    ((struct stat_uint64_t_counter_t*)counter)->min = UINT64_MAX;	\
    ((struct stat_uint64_t_counter_t*)counter)->max = 0;		\
  }

/* initialize a counter */
#define __init_stat_counter(type, counter)				\
  {									\
    stat_counter_t(type) * _c_ = (stat_counter_t(type) *) (counter);	\
    __init_##type##_stat_counter(_c_);					\
  }

/* update a counter */
#define __update_stat_counter(type, counter, new_value)			\
  {									\
    stat_counter_t(type) * _c_ = (stat_counter_t(type) *) (counter);	\
    _c_ -> sum += (new_value);						\
    if((new_value) < _c_->min)						\
      _c_->min = (new_value);						\
    if((new_value) > _c_->max)						\
      _c_->max = (new_value);						\
  }

/* reduce a counter */
#define __reduce_stat_counter(type, src_counter, dest_counter)		\
  {									\
    stat_counter_t(type) *_src_ = (stat_counter_t(type) *)(src_counter); \
    stat_counter_t(type) *_dest_ = (stat_counter_t(type) *)(dest_counter); \
									\
    _dest_->sum += _src_->sum;						\
									\
    if(_src_->min < _dest_->min)					\
      _dest_->min = _src_->min;						\
									\
    if(_src_->max > _dest_->max)					\
      _dest_->max = _src_->max;						\
  }

#endif	/* EZTRACE_STATS_CORE_H */
