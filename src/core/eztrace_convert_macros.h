/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_CONVERT_MACROS_H
#define EZTRACE_CONVERT_MACROS_H

#include <stdint.h>

#define VERBOSE (*get_verbose())
#define NB_TRACES (*get_nb_traces())
#define NB_START (*get_nb_start())
#define CUR_TRACE (get_cur_trace())
#define CUR_EV (get_cur_ev())
#define SKIP (*get_skip())


#define STARTED (CUR_TRACE->start != 0)

#if TID_RECORDING_ENABLED
#define CUR_THREAD_ID CUR_TRACE->ev.user.tid

#define GET_PARAM(p_ev, num_param) (p_ev)->param[(num_param)-1]
#define GET_NBPARAMS(p_ev) (p_ev)->nb_params
#else
#define CUR_THREAD_ID CUR_TRACE->ev.param[0]
#define GET_PARAM(p_ev, num_param) (p_ev)->param[(num_param)]
#define GET_NBPARAMS(p_ev) (p_ev)->nb_params-1
#endif

#define CURRENT (double)(((CUR_TRACE->ev.time - CUR_TRACE->start_time)+CUR_TRACE->delay)/1000000.0)
#define CUR_TIME(i) (uint64_t)((get_traces(i)->ev.time - get_traces(i)->start_time) + get_traces(i)->delay)
#define CUR_RANK (CUR_TRACE->rank)
#define CUR_ID (CUR_TRACE->trace_id)
#define CUR_INDEX (CUR_TRACE->id)

#define FUNC_NAME __func_name(CUR_TRACE->id, CURRENT, __FUNCTION__)

#define CREATE_TRACE_ID_STR(__var__, __trace_index__)	\
  asprintf (&(__var__), "P#%d", __trace_index__)

/* create the process id string */
#define CREATE_PROCESS_ID_STR(__var__, __trace_id__)	\
  asprintf (&(__var__), "%s", __trace_id__)

/* create the thread id string */
#define CREATE_THREAD_ID_STR(__var__, __trace_id__, __tid__)	\
  asprintf (&(__var__), "%s_T#%u", __trace_id__, __tid__)


/* create the process name string */
#define CREATE_PROCESS_NAME_STR(__var__, __trace_index__)	\
  asprintf (&(__var__), "CT_Process #%d", __trace_index__)

/* create the thread name string */
#define CREATE_THREAD_NAME_STR(__var__, __trace_id__, __tid__)	\
  asprintf (&(__var__), "%s_T#%llu", __trace_id__, __tid__)


static inline struct eztrace_container_t * get_thread_cont_from_id(int trace_id, int thread_id);

/* get a pointer to container */
static inline struct eztrace_container_t*
GET_ROOT_CONTAINER(int trace_index) {
  struct trace_t* ret = get_traces(trace_index);
  if(ret)
    return &ret->root_container;
  return NULL;
}

#define GET_PROCESS_CONTAINER(__trace_index__) GET_ROOT_CONTAINER(__trace_index__)
#define GET_THREAD_CONTAINER(__trace_index__, __thread_id__) get_thread_cont_from_id(__trace_index__, __thread_id__)

/* get a pointer to the info structure */
static inline
struct process_info_t* GET_PROCESS_INFO(int trace_index) {
  struct eztrace_container_t* ret = GET_PROCESS_CONTAINER(trace_index);
  if(ret)
    return (struct process_info_t*)(ret->container_info);
  return NULL;
}

static inline
struct thread_info_t* GET_THREAD_INFO(int trace_index, int thread_id) {
  struct eztrace_container_t* ret = GET_THREAD_CONTAINER(trace_index, thread_id);
  if(ret)
    return (struct thread_info_t*)(ret->container_info);
  return NULL;
}

/* get a pointer to the id string */
static inline char*
GET_PROCESS_ID_STR(int trace_index) {
  struct eztrace_container_t* ret = GET_ROOT_CONTAINER(trace_index);
  if(ret)
    return ret->id;
  return NULL;
};

static inline char*
GET_THREAD_ID_STR(int trace_index, int thread_id) {
  struct eztrace_container_t* ret = GET_THREAD_CONTAINER(trace_index, thread_id);
  if(ret)
    return ret->id;
  return NULL;
};

/* get a pointer to the name string */
static inline char*
GET_PROCESS_NAME_STR(int trace_index) {
  struct eztrace_container_t* ret = GET_ROOT_CONTAINER(trace_index);
  if(ret)
    return ret->name;
  return NULL;
};

static inline char*
GET_THREAD_NAME_STR(int trace_index, int thread_id) {
  struct eztrace_container_t* ret = GET_THREAD_CONTAINER(trace_index, thread_id);
  if(ret)
    return ret->name;
  return NULL;
};

/* Same as above, but the variable is declared. */
#define DECLARE_PROCESS_INFO(__var__, __trace_index__)			\
  struct process_info_t* __var__ = GET_PROCESS_INFO(__trace_index__);

#define DECLARE_THREAD_INFO(__var__, __trace_index__, __thread_id__)	\
  struct thread_info_t* __var__ = GET_THREAD_INFO(__trace_index__, __thread_id__)

#define DECLARE_PROCESS_ID_STR(__var__, __trace_index__)	\
  char* __var__ = GET_PROCESS_ID_STR(__trace_index__)
#define DECLARE_PROCESS_NAME_STR(__var__, __trace_index__)	\
  char* __var__ = GET_PROCESS_NAME_STR(__trace_index__)

#define DECLARE_THREAD_ID_STR(__var__, __trace_index__, __thread_id__)	\
  char* __var__ = GET_THREAD_ID_STR(__trace_index__, __thread_id__)
#define DECLARE_THREAD_NAME_STR(__var__, __trace_index__, __thread_id__) \
  char* __var__ = GET_THREAD_NAME_STR(__trace_index__, __thread_id__)

/* declare a var variable that points to the current process */
#define DECLARE_CUR_PROCESS(var)				\
  struct process_info_t * var = GET_PROCESS_INFO(CUR_INDEX);	\
  { if(!(var)) handle_new_thread (); }

/* declare a var variable that points to the current thread */
#define DECLARE_CUR_THREAD(var)						\
  struct thread_info_t * var = GET_THREAD_INFO(CUR_INDEX, CUR_THREAD_ID); \
  {									\
    if(!(var)) {							\
      handle_new_thread ();						\
      var = GET_THREAD_INFO(CUR_INDEX, CUR_THREAD_ID);			\
    }									\
  }

/* Use the CHANGE macro before any trace function that modifies a thread/process state.
 * Otherwise, when MPI_Init is grabbed and the timer reset, the trace will be a mess.
 */
#define CHANGE() if(CUR_TRACE->start)



/* create an identifier for a specific application pointer
 * This identifier is only valid within the current process.
 */
#define CREATE_OBJECT_ID(varname, ptr)					\
  char *varname;							\
  {									\
    int __attribute__((unused))						\
      ret = asprintf (&varname,						\
		      "%s_ptr_%p",					\
		      GET_PROCESS_ID_STR(CUR_INDEX),			\
		      (app_ptr) ptr);					\
  }

#endif	/* EZTRACE_CONVERT_MACROS_H */
