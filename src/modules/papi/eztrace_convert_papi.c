/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <GTG.h>

#include "eztrace_convert.h"
#include "papi_ev_codes.h"
#include "eztrace_list.h"

static int recording_stats = 0;

#define PAPI_CHANGE() if(!recording_stats) CHANGE()

static int nb_papi_counters = 0;

struct __papi_counter_info{
  int counter_code;
  char* gtg_alias;
  char* gtg_desc;
  uint64_t sum_counters;
  double total_duration;
};

static struct __papi_counter_info *counters_info;

void handle_papi_event()
{
  if(!STARTED)
    return 0;

 FUNC_NAME;

 DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
 int index = GET_PARAM(CUR_EV, 1);
 int counter_diff = GET_PARAM(CUR_EV, 2);
 double duration = (GET_PARAM(CUR_EV, 3))/1e6;

 counters_info[index].sum_counters += counter_diff;
 counters_info[index].total_duration += duration;
 PAPI_CHANGE() setVar(CURRENT, counters_info[index].gtg_alias, thread_id, counter_diff/duration);
}

void handle_papi_init()
{
 FUNC_NAME;

 /* if using MPI + PAPI, papi is initialized once per process */
 static int already_initialized = 0;

 int i;
 int nb_counters = GET_PARAM(CUR_EV, 1);
 if(already_initialized) {
   /* make sure all the processes use the same counters */
   assert(nb_counters == nb_papi_counters);
 }

 nb_papi_counters = nb_counters;
 if(!already_initialized)
   counters_info = malloc(nb_papi_counters*sizeof(struct __papi_counter_info));
 /* if using MPI, we still need to process the following events (the ones that define the PAPI
  * codes to use).
  */
 for(i=0; i<nb_papi_counters; i++) {
   wait_for_an_event(CUR_TRACE->id, FUT_PAPI_INIT_COUNTERS);

   if(already_initialized) {
     /* make sure all the processes use the same counters */
     assert(counters_info[i].counter_code == GET_PARAM(CUR_EV, 2));
     /* we don't need to do the remaining */
     continue;
   }

   counters_info[i].counter_code = GET_PARAM(CUR_EV, 2);

   int id = __papi_get_counter_id_by_code(counters_info[i].counter_code);
   if(id == -1) {
     __papi_print_counter_ids();
     fprintf(stderr, "Unknown PAPI counter code: %x\n", counters_info[i].counter_code);
   }
   counters_info[i].gtg_desc = NULL;
   asprintf(&(counters_info[i].gtg_desc), "%s per second", papi_counter_ids[id].description);

   counters_info[i].gtg_alias = papi_counter_ids[id].code_str;
   counters_info[i].sum_counters = 0;
   counters_info[i].total_duration = 0;

  if(get_mode() == EZTRACE_CONVERT) {
    addVarType (counters_info[i].gtg_alias, counters_info[i].gtg_desc, "CT_Thread");
  }
 }

 if(nb_papi_counters)
   already_initialized = 1;
}


int
eztrace_convert_papi_init()
{
  if(get_mode() == EZTRACE_CONVERT) {
    addVarType ("V_L3miss", "L3 Misses", "CT_Thread");
  }
  return 0;
}

/* return 1 if the event was handled */
int
handle_papi_events(struct fxt_ev_64 *ev)
{
  switch(ev->code)
    {
    case FUT_PAPI_INIT:
      handle_papi_init(); break;
    case FUT_PAPI_MEASUREMENT:
      handle_papi_event(); break;
    default:
      return 0;
    }
  return 1;
}

int
handle_papi_stats(struct fxt_ev_64 *ev)
{
  recording_stats = 1;
  return handle_papi_events(ev);
}

void print_papi_stats() {
  printf ( "\nPAPI:\n");
  printf ( "-------\n");

  int i;
  for(i=0;i<nb_papi_counters; i++) {

    printf ( "average %s: %lf\n", counters_info[i].gtg_desc,
	     counters_info[i].sum_counters/counters_info[i].total_duration);
  }
}

struct eztrace_convert_module papi_module;

void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  papi_module.api_version = EZTRACE_API_VERSION;
  papi_module.init = eztrace_convert_papi_init;
  papi_module.handle = handle_papi_events;
  papi_module.handle_stats = handle_papi_stats;
  papi_module.print_stats = print_papi_stats;

  papi_module.module_prefix = PAPI_EVENTS_ID;
  asprintf(&papi_module.name, "papi");
  asprintf(&papi_module.description, "Module for PAPI Performance counters");

  papi_module.token.data = &papi_module;
  eztrace_convert_register_module(&papi_module);

  int ret = PAPI_library_init(PAPI_VER_CURRENT);
  if (ret != PAPI_VER_CURRENT && ret > 0) {
    fprintf(stderr,"PAPI library version mismatch!\n");
    exit(1);
  }

  __papi_init_counter_ids();
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
}
