#include "eztrace.h"

#include "herbert_ev_codes.h"

int (*libherbertvonkrugell) (int toto);



int herbertvonkrugell (int toto) {
	EZTRACE_EVENT1 (FUT_herbert_herbertvonkrugell_1, toto);
	int ret = libherbertvonkrugell (toto);
	EZTRACE_EVENT1 (FUT_herbert_herbertvonkrugell_2, toto);
	return ret;
}



START_INTERCEPT
  INTERCEPT2("herbertvonkrugell", libherbertvonkrugell)


END_INTERCEPT

static void __herbert_init (void) __attribute__ ((constructor));
/* Initialize the current library */
static void
__herbert_init (void)
{
  DYNAMIC_INTERCEPT_ALL();

  /* start event recording */
#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#endif
}

static void __herbert_conclude (void) __attribute__ ((destructor));
static void
__herbert_conclude (void)
{
  /* stop event recording */
  eztrace_stop ();
}
