#include <stdint.h>
#include <stdio.h>
#include "instrument.h"

static uint64_t nb_events = 0;

void record_event0(uint64_t code)
{
  //printf("record_event0\n");
  nb_events++;
}

void record_event1(uint64_t code, uint64_t arg1)
{
  nb_events++;
}

void record_event2(uint64_t code, uint64_t arg1, uint64_t arg2)
{
  nb_events++;
}

void record_event3(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
  nb_events++;
}

void record_event4(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
  nb_events++;
}

void record_event5(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5)
{
  nb_events++;
}

void __init (void) __attribute__ ((constructor));
void __init(void)
{
	fprintf(stderr,  "Starting instrument-dummy...\n");
}

void __conclude (void) __attribute__ ((destructor));
void
__conclude (void)
{
  fprintf (stderr, "Stopping instrument-dummy...\n");
  fprintf (stderr, "\t%ld events\n", nb_events);
}
