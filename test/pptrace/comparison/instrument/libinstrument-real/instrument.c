/* don't forget this line, otherwise, FUT_DO_PROBE0 is defined to (void) 0 */
#define CONFIG_FUT

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include "fxt/fxt.h"
#include "fxt/fut.h"

#include "instrument.h"


static int PROF_BUFFER_SIZE = (128*1024*1024);

static uint64_t nb_events = 0;

#define EZTRACE_EVENT0(code) FUT_DO_PROBE0(code)
/* create an event with one value */
#define EZTRACE_EVENT1(code, arg1) FUT_DO_PROBE1(code, arg1)
/* create an event with two value */
#define EZTRACE_EVENT2(code, arg1, arg2) FUT_DO_PROBE2(code, arg1, arg2)
/* etc. */
#define EZTRACE_EVENT3(code, arg1, arg2, arg3) FUT_DO_PROBE3(code, arg1, arg2, arg3)
#define EZTRACE_EVENT4(code, arg1, arg2, arg3, arg4) FUT_DO_PROBE4(code, arg1, arg2, arg3, arg4)
#define EZTRACE_EVENT5(code, arg1, arg2, arg3, arg4, arg5) FUT_DO_PROBE5(code, arg1, arg2, arg3, arg4, arg5)

void record_event0(uint64_t code)
{
  EZTRACE_EVENT0(code);
}

void record_event1(uint64_t code, uint64_t arg1)
{
  EZTRACE_EVENT1(code, arg1);
}

void record_event2(uint64_t code, uint64_t arg1, uint64_t arg2)
{
  EZTRACE_EVENT2(code, arg1, arg2);
}

void record_event3(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
  EZTRACE_EVENT3(code, arg1, arg2, arg3);
}

void record_event4(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
  EZTRACE_EVENT4(code, arg1, arg2, arg3, arg4);
}

void record_event5(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
  EZTRACE_EVENT5(code, arg1, arg2, arg3, arg4, arg5);
}


static char PROF_FILE_USER[1024];
static int initialized = 0;

void __init (void) __attribute__ ((constructor));
void
__init (void)
{
  fprintf (stderr, "Starting FxT instrumentation...\n");
  if(initialized)
    return;


  sprintf(PROF_FILE_USER, "/var/tmp/%s_trace", getenv("USER"));

  //fut_enable_tid_logging();
  /* start FxT */
  if (fut_setup (PROF_BUFFER_SIZE, FUT_KEYMASKALL, syscall(SYS_gettid)) < 0)
    {
      perror ("fut_setup");
      exit (EXIT_FAILURE);
    }

  fprintf(stderr, "Init Done\n");
  initialized = 1;
}

void __conclude (void) __attribute__ ((destructor));
void
__conclude (void)
{
  fprintf (stderr, "Stopping FxT instrumentation...\n");
  if (!initialized){
    printf("nope\n");
    return;
  }

  initialized = 0;

  fut_endup (PROF_FILE_USER);
  fut_done ();
  fprintf (stderr, "Stopping FxT instrumentation... saving trace  %s\n", PROF_FILE_USER);
}
