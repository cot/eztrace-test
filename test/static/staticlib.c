#include "eztrace.h"
#include "staticlib_ev_codes.h"

int (*libmafunc) (int n);



int mafunc (int n) {
	EZTRACE_EVENT1 (FUT_staticlib_mafunc_1, n);
	int ret = libmafunc (n);
	EZTRACE_EVENT1 (FUT_staticlib_mafunc_2, n);
	return ret;
}



START_INTERCEPT
  INTERCEPT2("mafunc", libmafunc)
END_INTERCEPT

static void __staticlib_init (void) __attribute__ ((constructor));
/* Initialize the current library */
static void
__staticlib_init (void)
{
  DYNAMIC_INTERCEPT_ALL();

  /* start event recording */
#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#endif
}

static void __staticlib_conclude (void) __attribute__ ((destructor));
static void
__staticlib_conclude (void)
{
  /* stop event recording */
  eztrace_stop ();
}
