/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE 1
#define _REENTRANT

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#include "pomp2_lib.h"
#include "gomp_ev_codes.h"
#include "eztrace.h"
#include "pptrace.h"

static int pomp2_found = 0;
#define GOMP_RECORD if(!pomp2_found)

// todo: add hooks for
// OMP_Barrier
// OMP_critical ?
void (*libGOMP_parallel_loop_static_start)(void (*)(void *), void *, unsigned, long, long, long, long);
void (*libGOMP_parallel_loop_runtime_start)(void (*)(void *), void *, unsigned, long, long, long, long);
void (*libGOMP_parallel_loop_dynamic_start)(void (*)(void *), void *, unsigned, long, long, long, long);
void (*libGOMP_parallel_loop_guided_start)(void (*)(void *), void *, unsigned, long, long, long, long);

void (*libGOMP_parallel_start)(void (*fn)(void *), void *data, unsigned num_threads);
void (*libGOMP_parallel_end)();

void (*libGOMP_critical_start)(void);
void (*libGOMP_critical_end)(void);

struct gomp_arg_t {
    void (*func)(void *);
    void *data;
    int id;
};

/* Function called by GOMP_parallel_start for each thread */
void gomp_new_thread(void *arg) {
    FUNCTION_ENTRY;
    struct gomp_arg_t *_arg = (struct gomp_arg_t*) arg;
    void (*func)(void *) = _arg->func;
    void *data = _arg->data;
    int section_id = _arg->id;
    int nb_threads = omp_get_num_threads();
    int my_id = omp_get_thread_num();
    /* Since the runtime functions provide more information, let's use it instead of the compiler functions */
    EZTRACE_EVENT3(FUT_GOMP_NEW_FORK, section_id, my_id, nb_threads);
    func(data);
    EZTRACE_EVENT1(FUT_GOMP_NEW_JOIN, my_id);
    return;
}

static int _next_section_id = 0;

/* generic implementation of parallel loop
 */
#define GOMP_PARALLEL_LOOP_GENERIC(fn, data, varname, gomp_func)	\
  {									\
    FUNCTION_ENTRY;							\
    int section_id = _next_section_id++;				\
    EZTRACE_PROTECT_ON();						\
    /* Since the runtime functions provide more information, let's use it instead of the compiler functions */ \
    /* call unprotected function 'cause the block is protected */ \
    EZTRACE_EVENT4_UNPROTECTED(FUT_GOMP_PARALLEL_START, fn, data, num_threads, section_id); \
    struct gomp_arg_t *varname = (struct gomp_arg_t*) malloc (sizeof (struct gomp_arg_t)); \
    varname->func = fn;							\
    varname->data = data;						\
    varname->id = section_id;						\
    EZTRACE_PROTECT_OFF();						\
    gomp_func;								\
    int nb_threads = omp_get_num_threads();				\
    int my_id = omp_get_thread_num();					\
    EZTRACE_EVENT3 (FUT_GOMP_NEW_FORK, section_id, my_id, nb_threads);  \
    return;								\
  }

/* should be called when reaching #pragma omp parallel for schedule(static)
 * However, this function doesn't seem to be called. Let's implement it just in case.
 */
void GOMP_parallel_loop_static_start(void (*fn)(void *), void * data, unsigned num_threads, long a1, long a2, long a3,
        long a4) {
    GOMP_PARALLEL_LOOP_GENERIC(fn, data, arg,
            libGOMP_parallel_loop_static_start (gomp_new_thread, arg, num_threads, a1, a2, a3, a4));
}

/* Function called when reaching  #pragma omp parallel for schedule(runtime) */
void GOMP_parallel_loop_runtime_start(void (*fn)(void *), void * data, unsigned num_threads, long a1, long a2, long a3,
        long a4) {
    GOMP_PARALLEL_LOOP_GENERIC(fn, data, arg,
            libGOMP_parallel_loop_runtime_start (gomp_new_thread, arg, num_threads, a1, a2, a3, a4));
}

/* Function called when reaching  #pragma omp parallel for schedule(dynamic) */
void GOMP_parallel_loop_dynamic_start(void (*fn)(void *), void * data, unsigned num_threads, long a1, long a2, long a3,
        long a4) {
    GOMP_PARALLEL_LOOP_GENERIC(fn, data, arg,
            libGOMP_parallel_loop_dynamic_start (gomp_new_thread, arg, num_threads, a1, a2, a3, a4));
}

/* Function called when reaching  #pragma omp parallel for schedule(guided) */
void GOMP_parallel_loop_guided_start(void (*fn)(void *), void * data, unsigned num_threads, long a1, long a2, long a3,
        long a4) {
    GOMP_PARALLEL_LOOP_GENERIC(fn, data, arg,
            libGOMP_parallel_loop_guided_start (gomp_new_thread, arg, num_threads, a1, a2, a3, a4));
}

// Called by the main thread (ie. only once) during #pragma omp parallel
// (fork)
void GOMP_parallel_start(void (*fn)(void *), void *data, unsigned num_threads) {
    GOMP_PARALLEL_LOOP_GENERIC(fn, data, arg, libGOMP_parallel_start (gomp_new_thread, arg, num_threads));
}

// Called at the end of a parallel section (~ join)
void GOMP_parallel_end() {
    FUNCTION_ENTRY;
    /* Since the runtime functions provide more information, let's use it instead of the compiler functions */
    int my_id = omp_get_thread_num();
    EZTRACE_EVENT1(FUT_GOMP_NEW_JOIN, my_id);
    libGOMP_parallel_end();
    EZTRACE_EVENT0(FUT_GOMP_JOIN_DONE);
}

void GOMP_critical_start() {
    FUNCTION_ENTRY;
    GOMP_RECORD
        EZTRACE_EVENT0(FUT_GOMP_CRITICAL_START);
    libGOMP_critical_start();
    GOMP_RECORD
        EZTRACE_EVENT0(FUT_GOMP_CRITICAL_START_DONE);
}

void GOMP_critical_end() {
    FUNCTION_ENTRY;
    GOMP_RECORD
        EZTRACE_EVENT0(FUT_GOMP_CRITICAL_STOP);
    libGOMP_critical_end();
}

/* beginning of pomp2 internals */

void POMP2_Finalize() {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_FINALIZE);
}

void POMP2_Init() {
    /* todo: initialize stuff ? */
}

void POMP2_Off() {
    /* todo: stop recording events ? */
}

void POMP2_On() {
    /* todo: initialize stuff ? */
}

void POMP2_Begin(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    /** Called at the begin of a user defined POMP2 region.
     @param pomp2_handle  The handle of the started region.
     */

}

void POMP2_End(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
}

void POMP2_Assign_handle(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    /** Registers a POMP2 region and returns a region handle.

     @param pomp2_handle  Returns the handle for the newly registered region.
     @param ctc_string   A string containing the region data.
     */
}

/* end of pomp2 internals */

void POMP2_Atomic_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    /** Called before an atomic statement.

     @param pomp2_handle The handle of the started region.
     @param ctc_string   Initialization string. May be ignored if \<pomp2_handle\> is already initialized.
     */
    FUNCTION_ENTRY;
    /* todo: record the initialization string ? */
    EZTRACE_EVENT0(FUT_POMP2_ATOMIC_ENTER);

}

void POMP2_Atomic_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_ATOMIC_EXIT);
}

void POMP2_Implicit_barrier_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle* pomp2_old_task __attribute__((unused))) {
    /** Called before an implicit barrier.

     \e OpenMP \e 3.0: Barriers can be used as scheduling points for
     tasks. When entering a barrier the task id of the currently
     executing task (\e pomp2_current_task) is saved in \e
     pomp2_old_task, which is defined inside the instrumented user
     code.

     @param pomp2_handle   The handle of the started region.
     @param pomp2_old_task Pointer to a "taskprivate" variable where the current task id is stored.
     */

    /* todo: implement */
}

extern void POMP2_Implicit_barrier_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_old_task __attribute__((unused))) {
    /* todo: implement */
}

void POMP2_Barrier_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle* pomp2_old_task __attribute__((unused)), const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_BARRIER_ENTER);
}

void POMP2_Barrier_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_old_task __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_BARRIER_EXIT);
}

void POMP2_Flush_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_FLUSH_ENTER);
}

void POMP2_Flush_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_FLUSH_EXIT);
}

void POMP2_Critical_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_CRITICAL_BEGIN);
}

void POMP2_Critical_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_CRITICAL_END);
}

void POMP2_Critical_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_CRITICAL_ENTER);
}

void POMP2_Critical_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_CRITICAL_EXIT);
}

void POMP2_For_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_FOR_ENTER);
}

void POMP2_For_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_FOR_EXIT);
}

void POMP2_Master_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_MASTER_BEGIN);
}

void POMP2_Master_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_MASTER_END);
}

void POMP2_Parallel_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_PARALLEL_BEGIN);
}

void POMP2_Parallel_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_PARALLEL_END);
}

void POMP2_Parallel_fork(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        int if_clause __attribute__((unused)), int num_threads __attribute__((unused)),
        POMP2_Task_handle* pomp2_old_task __attribute__((unused)), const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    int section_id = _next_section_id++;
    EZTRACE_EVENT1(FUT_POMP2_PARALLEL_FORK, section_id);
}

void POMP2_Parallel_join(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_old_task __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_PARALLEL_JOIN);
}

void POMP2_Section_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SECTION_BEGIN);
}

void POMP2_Section_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SECTION_END);
}

void POMP2_Sections_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SECTIONS_ENTER);
}

void POMP2_Sections_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SECTIONS_EXIT);
}

void POMP2_Single_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SINGLE_BEGIN);
}

void POMP2_Single_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SINGLE_END);
}

void POMP2_Single_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SINGLE_ENTER);
}

void POMP2_Single_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_SINGLE_EXIT);
}

void POMP2_Workshare_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_WORKSHARE_ENTER);
}

void POMP2_Workshare_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_WORKSHARE_EXIT);
}

void POMP2_Ordered_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    /** Called at the start of an ordered region.
     @param pomp2_handle  The handle of the region.
     */
    FUNCTION_ENTRY;
    // todo :   EZTRACE_EVENT0(FUT_POMP2_ORDERED_BEGIN);
}

void POMP2_Ordered_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    /* todo: implement that */
}

void POMP2_Ordered_enter(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    /* todo: implement that */
}

void POMP2_Ordered_exit(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    /* todo: implement that */
}

/** \e OpenMP \e 3.0: When a task encounters a task construct it creates
 a new task. The task may be scheduled for later execution or
 executed immediately. In both cases the pomp-adapter assigns the
 id of the currently active task to \e pomp2_old_task which is
 defined in the instrumented user code.

 @param pomp2_handle   The handle of the region.
 @param pomp2_old_task Pointer to the task id in the instrumented user code
 @param pomp2_if       If an if clause is present on the task
 directive this variable holds the evaluated
 result of the argument of the if
 clause. Else it is 1.
 @param ctc_string     The initialization string.

 */
void POMP2_Task_create_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle* pomp2_new_task __attribute__((unused)),
        POMP2_Task_handle* pomp2_old_task __attribute__((unused)), int pomp2_if __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    /* we could use pomp2_new_task to store the task id, but this would require a mutex
     * (or at least an atomic increment primitive) that would kill the performance.
     * In order to keep the overhead as low as possible, let's only record the event.
     */
    EZTRACE_EVENT0(FUT_POMP2_TASK_CREATE_BEGIN);
}

void POMP2_Task_create_end(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_old_task __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_TASK_CREATE_END);
}

void POMP2_Task_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_task __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_TASK_BEGIN);
}

void POMP2_Task_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_TASK_END);
}

void POMP2_Untied_task_create_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle* pomp2_new_task __attribute__((unused)),
        POMP2_Task_handle* pomp2_old_task __attribute__((unused)), int pomp2_if __attribute__((unused)),
        const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    /* we could use pomp2_new_task to store the task id, but this would require a mutex
     * (or at least an atomic increment primitive) that would kill the performance.
     * In order to keep the overhead as low as possible, let's only record the event.
     */
    EZTRACE_EVENT0(FUT_POMP2_UNTIED_TASK_CREATE_BEGIN);
}

void POMP2_Untied_task_create_end(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_old_task __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_UNTIED_TASK_CREATE_END);
}

void POMP2_Untied_task_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_parent_task __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_UNTIED_TASK_BEGIN);
}

void POMP2_Untied_task_end(POMP2_Region_handle* pomp2_handle __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_UNTIED_TASK_END);
}

void POMP2_Taskwait_begin(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle* pomp2_old_task __attribute__((unused)), const char ctc_string[] __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_TASKWAIT_BEGIN);
}

void POMP2_Taskwait_end(POMP2_Region_handle* pomp2_handle __attribute__((unused)),
        POMP2_Task_handle pomp2_old_task __attribute__((unused))) {
    FUNCTION_ENTRY;
    EZTRACE_EVENT0(FUT_POMP2_TASKWAIT_END);
}

#ifdef OPENMP_FOUND

void
POMP2_Init_lock( omp_lock_t* s )
{
    omp_init_lock(s);
}

void
POMP2_Destroy_lock( omp_lock_t* s )
{
    omp_destroy_lock(s);
}

void
POMP2_Set_lock( omp_lock_t* s )
{
    FUNCTION_ENTRY;
    EZTRACE_EVENT1(FUT_POMP2_SET_LOCK_ENTRY, s);
    omp_set_lock(s);
    EZTRACE_EVENT1(FUT_POMP2_SET_LOCK_EXIT, s);
}

void
POMP2_Unset_lock( omp_lock_t* s )
{
    FUNCTION_ENTRY;
    EZTRACE_EVENT1(FUT_POMP2_UNSET_LOCK, s);
    omp_unset_lock(s);
}

int
POMP2_Test_lock( omp_lock_t* s )
{
    int ret = omp_test_lock(s);
    if(ret) {
        FUNCTION_ENTRY;
        EZTRACE_EVENT1(FUT_POMP2_TEST_LOCK_SUCCESS, s);
    }
    return ret;
}

void
POMP2_Init_nest_lock( omp_nest_lock_t* s )
{
    omp_init_nest_lock(s);
}

void
POMP2_Destroy_nest_lock( omp_nest_lock_t* s )
{
    omp_destroy_nest_lock(s);
}

void
POMP2_Set_nest_lock( omp_nest_lock_t* s )
{
    FUNCTION_ENTRY;
    EZTRACE_EVENT1(FUT_POMP2_SET_NEST_LOCK_ENTRY, s);
    omp_set_nest_lock(s);
    EZTRACE_EVENT1(FUT_POMP2_SET_NEST_LOCK_EXIT, s);
}

void
POMP2_Unset_nest_lock( omp_nest_lock_t* s )
{
    FUNCTION_ENTRY;
    EZTRACE_EVENT1(FUT_POMP2_UNSET_NEST_LOCK, s);
    omp_unset_nest_lock(s);
}

int
POMP2_Test_nest_lock( omp_nest_lock_t* s )
{
    int ret = omp_test_nest_lock(s);
    if(ret) {
        FUNCTION_ENTRY;
        EZTRACE_EVENT1(FUT_POMP2_TEST_NEST_LOCK_SUCCESS, s);
    }
    return ret;
}

#endif	/* OPENMP_FOUND */

void (*libPOMP2_Finalize)();

START_INTERCEPT INTERCEPT2("GOMP_parallel_start", libGOMP_parallel_start)
INTERCEPT2("GOMP_parallel_end", libGOMP_parallel_end)
INTERCEPT2("GOMP_parallel_loop_static_start", libGOMP_parallel_loop_static_start)
INTERCEPT2("GOMP_parallel_loop_runtime_start", libGOMP_parallel_loop_runtime_start)
INTERCEPT2("GOMP_parallel_loop_dynamic_start", libGOMP_parallel_loop_dynamic_start)
INTERCEPT2("GOMP_parallel_loop_guided_start", libGOMP_parallel_loop_guided_start)

INTERCEPT2("GOMP_critical_start", libGOMP_critical_start)
INTERCEPT2("GOMP_critical_end", libGOMP_critical_end)
END_INTERCEPT
void __gomp_init(void) __attribute__ ((constructor));
void __gomp_init(void) {

    DYNAMIC_INTERCEPT_ALL();
    /* This symbol is only available if the program was compiled with eztrace_cc.
     * Use this information as a test and print a warning message.
     */
    INTERCEPT("POMP2_Finalize", libPOMP2_Finalize);
    if (libGOMP_critical_end && !libPOMP2_Finalize) {
        printf(
                "Only GNU OpenMP runtime functions will be intercepted. For a more precise trace, please instrument your program with eztrace_cc.\n");
    }

    if (libPOMP2_Finalize)
        pomp2_found = 1;
    else
        pomp2_found = 0;

#ifdef EZTRACE_AUTOSTART
    eztrace_start ();
#endif
}

void __gomp_conclude(void) __attribute__ ((destructor));
void __gomp_conclude(void) {
    eztrace_stop();
}
