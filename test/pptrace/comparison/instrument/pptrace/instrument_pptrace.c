#include <stdint.h>
#include <stdio.h>
#include "eztrace.h"
#include "instrument.h"

static int __foo_initialized = 0;
static int nb_events = 0;

int (*libfoo_local)() = NULL;
int (*libfoo)() = NULL;


int foo() {
  record_event0(1);
  int ret = libfoo();
  record_event0(2);
  return ret;
}


START_INTERCEPT
INTERCEPT2("foo", libfoo)
END_INTERCEPT

static void __foo_init (void) __attribute__ ((constructor));
static void
__foo_init (void)
{
  DYNAMIC_INTERCEPT_ALL();
  fprintf(stderr, "libfoo = %p\n", libfoo);
}

