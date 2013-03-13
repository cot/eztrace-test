/*************************************************************************/
/* OPARI Version 1.1                                                     */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef POMP_LIB_H
#define POMP_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <omp.h>

struct ompregdescr {
  char* name;                  /* name of construct                     */
  char* sub_name;              /* optional: region name                 */
  int   num_sections;          /* sections only: number of sections     */
  char* file_name;             /* source code location                  */
  int   begin_first_line;      /* line number first line opening pragma */
  int   begin_last_line;       /* line number last  line opening pragma */
  int   end_first_line;        /* line number first line closing pragma */
  int   end_last_line;         /* line number last  line closing pragma */
  void* data;                  /* space for performance data            */
  struct ompregdescr* next;    /* for linking                           */
};

extern int POMP_MAX_ID;

extern struct ompregdescr* pomp_rd_table[];

extern void c_pomp_finalize_();
extern void c_pomp_init_();
extern void c_pomp_off_();
extern void c_pomp_on_();
extern void c_pomp_atomic_enter_(struct ompregdescr* r);
extern void c_pomp_atomic_exit_(struct ompregdescr* r);
extern void c_pomp_barrier_enter_(struct ompregdescr* r);
extern void c_pomp_barrier_exit_(struct ompregdescr* r);
extern void c_pomp_flush_enter_(struct ompregdescr* r);
extern void c_pomp_flush_exit_(struct ompregdescr* r);
extern void c_pomp_critical_begin_(struct ompregdescr* r);
extern void c_pomp_critical_end_(struct ompregdescr* r);
extern void c_pomp_critical_enter_(struct ompregdescr* r);
extern void c_pomp_critical_exit_(struct ompregdescr* r);
extern void c_pomp_task_begin_(struct ompregdescr* r);
extern void c_pomp_task_end_(struct ompregdescr* r);
extern void c_pomp_taskwait_enter_(struct ompregdescr* r);
extern void c_pomp_taskwait_exit_(struct ompregdescr* r);
extern void c_pomp_for_enter_(struct ompregdescr* r);
extern void c_pomp_for_exit_(struct ompregdescr* r);
extern void c_pomp_master_begin_(struct ompregdescr* r);
extern void c_pomp_master_end_(struct ompregdescr* r);
extern void c_pomp_parallel_begin_(struct ompregdescr* r);
extern void c_pomp_parallel_end_(struct ompregdescr* r);
extern void c_pomp_parallel_fork_(struct ompregdescr* r);
extern void c_pomp_parallel_join_(struct ompregdescr* r);
extern void c_pomp_section_begin_(struct ompregdescr* r);
extern void c_pomp_section_end_(struct ompregdescr* r);
extern void c_pomp_sections_enter_(struct ompregdescr* r);
extern void c_pomp_sections_exit_(struct ompregdescr* r);
extern void c_pomp_single_begin_(struct ompregdescr* r);
extern void c_pomp_single_end_(struct ompregdescr* r);
extern void c_pomp_single_enter_(struct ompregdescr* r);
extern void c_pomp_single_exit_(struct ompregdescr* r);
extern void c_pomp_workshare_enter_(struct ompregdescr* r);
extern void c_pomp_workshare_exit_(struct ompregdescr* r);
extern void c_pomp_begin_(struct ompregdescr* r);
extern void c_pomp_end_(struct ompregdescr* r);

#define pomp_finalize c_pomp_finalize_
#define pomp_init c_pomp_init_
#define pomp_off c_pomp_off_
#define pomp_on c_pomp_on_
#define pomp_atomic_enter c_pomp_atomic_enter_
#define pomp_atomic_exit c_pomp_atomic_exit_
#define pomp_barrier_enter c_pomp_barrier_enter_
#define pomp_barrier_exit c_pomp_barrier_exit_
#define pomp_flush_enter c_pomp_flush_enter_
#define pomp_flush_exit c_pomp_flush_exit_
#define pomp_critical_begin c_pomp_critical_begin_
#define pomp_critical_end c_pomp_critical_end_
#define pomp_critical_enter c_pomp_critical_enter_
#define pomp_critical_exit c_pomp_critical_exit_
#define pomp_task_begin c_pomp_task_begin_
#define pomp_task_end c_pomp_task_end_
#define pomp_taskwait_enter c_pomp_taskwait_enter_
#define pomp_taskwait_exit c_pomp_taskwait_exit_
#define pomp_for_enter c_pomp_for_enter_
#define pomp_for_exit c_pomp_for_exit_
#define pomp_master_begin c_pomp_master_begin_
#define pomp_master_end c_pomp_master_end_
#define pomp_parallel_begin c_pomp_parallel_begin_
#define pomp_parallel_end c_pomp_parallel_end_
#define pomp_parallel_fork c_pomp_parallel_fork_
#define pomp_parallel_join c_pomp_parallel_join_
#define pomp_section_begin c_pomp_section_begin_
#define pomp_section_end c_pomp_section_end_
#define pomp_sections_enter c_pomp_sections_enter_
#define pomp_sections_exit c_pomp_sections_exit_
#define pomp_single_begin c_pomp_single_begin_
#define pomp_single_end c_pomp_single_end_
#define pomp_single_enter c_pomp_single_enter_
#define pomp_single_exit c_pomp_single_exit_
#define pomp_workshare_enter c_pomp_workshare_enter_
#define pomp_workshare_exit c_pomp_workshare_exit_
#define pomp_begin c_pomp_begin_
#define pomp_end c_pomp_end_

extern void c_pomp_init_lock_(omp_lock_t *s);
extern void c_pomp_destroy_lock_(omp_lock_t *s);
extern void c_pomp_set_lock_(omp_lock_t *s);
extern void c_pomp_unset_lock_(omp_lock_t *s);
extern int  c_pomp_test_lock_(omp_lock_t *s);
extern void c_pomp_init_nest_lock_(omp_nest_lock_t *s);
extern void c_pomp_destroy_nest_lock_(omp_nest_lock_t *s);
extern void c_pomp_set_nest_lock_(omp_nest_lock_t *s);
extern void c_pomp_unset_nest_lock_(omp_nest_lock_t *s);
extern int  c_pomp_test_nest_lock_(omp_nest_lock_t *s);

#define pomp_init_lock c_pomp_init_lock_
#define pomp_destroy_lock c_pomp_destroy_lock_
#define pomp_set_lock c_pomp_set_lock_
#define pomp_unset_lock c_pomp_unset_lock_
#define pomp_test_lock c_pomp_test_lock_
#define pomp_init_nest_lock c_pomp_init_nest_lock_
#define pomp_destroy_nest_lock c_pomp_destroy_nest_lock_
#define pomp_set_nest_lock c_pomp_set_nest_lock_
#define pomp_unset_nest_lock c_pomp_unset_nest_lock_
#define pomp_test_nest_lock c_pomp_test_nest_lock_

extern int pomp_tracing;

#ifdef __cplusplus
}
#endif

#endif
