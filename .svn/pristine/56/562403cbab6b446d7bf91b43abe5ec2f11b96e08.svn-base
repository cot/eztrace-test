/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <GTG.h>

#include "eztrace_convert.h"
#include "memory_ev_codes.h"
#include "eztrace_list.h"

static int recording_stats = 0;

#define MEMORY_CHANGE() if(!recording_stats) CHANGE()

struct memory_process_info_t {
  struct process_info_t *p_process;

  uint64_t memory_use;
  uint32_t nb_malloc;
  uint32_t nb_free;
  uint64_t total_malloced;
  uint64_t total_freed;
  uint32_t max_mem_used;
};

static struct memory_process_info_t *__register_process_hook(struct process_info_t *p_process)
{
  struct memory_process_info_t *p_mem = (struct memory_process_info_t*) malloc(sizeof(struct memory_process_info_t));
  p_mem->p_process = p_process;
  p_mem->memory_use = 0;
  p_mem->nb_malloc = 0;
  p_mem->nb_free = 0;
  p_mem->total_malloced = 0;
  p_mem->total_freed = 0;
  p_mem->max_mem_used = 0;

  /* add the hook in the thread info structure */
  ezt_hook_list_add(&p_mem->p_process->hooks, p_mem, (uint8_t)MEMORY_EVENTS_ID);
  return p_mem;
}

#define  INIT_MEMORY_PROCESS_INFO(p_process, var)			\
  struct memory_process_info_t *var = (struct memory_process_info_t*)	\
    ezt_hook_list_retrieve_data(&p_process->hooks, (uint8_t)MEMORY_EVENTS_ID); \
  if(!(var)) {								\
    var = __register_process_hook(p_process);				\
  }

void handle_memory_malloc()
{
 FUNC_NAME;
 /* warning: this function may be called before the trace is started (eg. before MPI_Init is detected)
  * so proc_id may be undefined.
  * In this case, proc_id is used only after MPI_Init, so there's no real problem.
  */
 DECLARE_PROCESS_ID_STR(proc_id, CUR_INDEX);
 int block_size = GET_PARAM(CUR_EV, 1);
 //app_ptr block_start = GET_PARAM(CUR_EV, 2);

 DECLARE_CUR_PROCESS(p_process);
 INIT_MEMORY_PROCESS_INFO(p_process, p_mem);

 p_mem->nb_malloc++;
 p_mem->memory_use += block_size;
 p_mem->total_malloced += block_size;

 if(p_mem->memory_use > p_mem->max_mem_used)
   p_mem->max_mem_used = p_mem->memory_use;

 MEMORY_CHANGE() setVar(CURRENT, "V_Mem", proc_id, p_mem->memory_use);
}

void handle_memory_realloc()
{
 FUNC_NAME;
 /* warning: this function may be called before the trace is started (eg. before MPI_Init is detected)
  * so proc_id may be undefined.
  * In this case, proc_id is used only after MPI_Init, so there's no real problem.
  */
 DECLARE_PROCESS_ID_STR(proc_id, CUR_INDEX);
 int old_size = GET_PARAM(CUR_EV, 1);
 int new_size = GET_PARAM(CUR_EV, 2);
 //app_ptr block_start = GET_PARAM(CUR_EV, 3);

 DECLARE_CUR_PROCESS(p_process);
 INIT_MEMORY_PROCESS_INFO(p_process, p_mem);

 p_mem->nb_malloc++;
 p_mem->nb_free++;
 p_mem->memory_use += new_size - old_size;
 p_mem->total_malloced += new_size - old_size;

 if(p_mem->memory_use > p_mem->max_mem_used)
   p_mem->max_mem_used = p_mem->memory_use;

 MEMORY_CHANGE() setVar(CURRENT, "V_Mem", proc_id, p_mem->memory_use);
}

void handle_memory_free()
{
 FUNC_NAME;
 /* warning: this function may be called before the trace is started (eg. before MPI_Init is detected)
  * so proc_id may be undefined.
  * In this case, proc_id is used only after MPI_Init, so there's no real problem.
  */
 DECLARE_PROCESS_ID_STR(proc_id, CUR_INDEX);
 size_t block_size = GET_PARAM(CUR_EV, 1);
 //app_ptr block_start = GET_PARAM(CUR_EV, 2);

 DECLARE_CUR_PROCESS(p_process);
 INIT_MEMORY_PROCESS_INFO(p_process, p_mem);

 p_mem->nb_free++;
 if( p_mem->memory_use >= block_size)
   p_mem->memory_use -= block_size;
 else {
   fprintf(stderr, "t=%lf\tWarning: %s frees %ld bytes, but only %ld are currently allocated\n",
	   CURRENT, CUR_ID, block_size, p_mem->memory_use);
   p_mem->memory_use = 0;
 }

 p_mem->total_freed += block_size;

 MEMORY_CHANGE() setVar(CURRENT, "V_Mem", proc_id, p_mem->memory_use);
}

int
eztrace_convert_memory_init()
{
  if(get_mode() == EZTRACE_CONVERT) {
    addVarType ("V_Mem", "Memory used", "CT_Process");
  }
  return 0;
}

/* return 1 if the event was handled */
int
handle_memory_events(struct fxt_ev_64 *ev)
{
  switch(ev->code)
    {
    case FUT_MEMORY_MALLOC:
      handle_memory_malloc(); break;
    case FUT_MEMORY_FREE:
      handle_memory_free(); break;
    case FUT_MEMORY_REALLOC:
      handle_memory_realloc(); break;

    default:
      return 0;
    }
  return 1;
}

int
handle_memory_stats(struct fxt_ev_64 *ev)
{
  recording_stats = 1;
  return handle_memory_events(ev);
}

void print_memory_stats() {
  printf ( "\nMEMORY:\n");
  printf   ( "-------\n");


  int i;
  uint32_t nb_malloc = 0;
  uint32_t nb_free = 0;
  uint32_t total_malloced = 0;
  uint32_t total_freed = 0;
  uint32_t max_mem_used = 0;

  for(i=0; i<NB_TRACES; i++) {
    struct process_info_t *p_process = GET_PROCESS_INFO(i);
    struct memory_process_info_t *p_info = (struct memory_process_info_t*) ezt_hook_list_retrieve_data(&p_process->hooks, (uint8_t)MEMORY_EVENTS_ID);

    if(!p_info) {
      printf("No p_info for %s\n", p_process->container->name);
      continue;			/* No memory process info attached, skip this process */
    }

    nb_malloc += p_info->nb_malloc;
    nb_free += p_info->nb_free;
    total_malloced += p_info->total_malloced;
    total_freed += p_info->total_freed;

    if(p_info->max_mem_used > max_mem_used)
      max_mem_used = p_info->max_mem_used;


    printf("\t%s:\t", p_process->container->name);
    printf("%d malloc (total: %d bytes)\t", p_info->nb_malloc, p_info->total_malloced);
    printf("%d free (total: %d bytes)\t", p_info->nb_free, p_info->total_freed);
    printf("maximum memory used: %d bytes\n", p_info->max_mem_used);

  }

  printf("Total:\t");
  printf("%d malloc (total: %d bytes)\t", nb_malloc, total_malloced);
  printf("%d free (total: %d bytes)\t", nb_free, total_freed);
  printf("maximum memory used: %d bytes\n", max_mem_used);

}

struct eztrace_convert_module memory_module;

void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  memory_module.api_version = EZTRACE_API_VERSION;
  memory_module.init = eztrace_convert_memory_init;
  memory_module.handle = handle_memory_events;
  memory_module.handle_stats = handle_memory_stats;
  memory_module.print_stats = print_memory_stats;

  memory_module.module_prefix = MEMORY_EVENTS_ID;
  asprintf(&memory_module.name, "memory");
  asprintf(&memory_module.description, "Module for memory functions (malloc, free, etc.)");

  memory_module.token.data = &memory_module;
  eztrace_convert_register_module(&memory_module);
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
}
