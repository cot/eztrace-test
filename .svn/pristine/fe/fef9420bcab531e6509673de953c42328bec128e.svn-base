#include <stdio.h>
#include <GTG.h>
#include "eztrace_convert.h"
#include "staticlib_ev_codes.h"

int eztrace_convert_staticlib_init();

int handle_staticlib_events(struct fxt_ev_64 *ev);

void handleFUT_staticlib_mafunc_1() ;
void handleFUT_staticlib_mafunc_2() ;




/* Constructor of the plugin.
 * This function registers the current module to eztrace_convert
 */
struct eztrace_convert_module staticlib_module;
void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  staticlib_module.api_version = EZTRACE_API_VERSION;

  /* Specify the initialization function.
   * This function will be called once all the plugins are loaded
   * and the trace is started.
   * This function usually declared StateTypes, LinkTypes, etc.
   */
  staticlib_module.init = eztrace_convert_staticlib_init;

  /* Specify the function to call for handling an event
   */
  staticlib_module.handle = handle_staticlib_events;

  /* Specify the module prefix */
  staticlib_module.module_prefix = staticlib_EVENTS_ID;

  asprintf(&staticlib_module.name, "staticlib");
  asprintf(&staticlib_module.description, "module for the example library");

  staticlib_module.token.data = &staticlib_module;

  /* Register the module to eztrace_convert */
  eztrace_convert_register_module(&staticlib_module);

  //printf("module staticlib loaded\n");
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
  //printf("unloading module staticlib\n");
}



/*
 * This function will be called once all the plugins are loaded
 * and the trace is started.
 * This function usually declared StateTypes, LinkTypes, etc.
 */
int
eztrace_convert_staticlib_init()
{
    addEntityValue("staticlib_STATE_0", "ST_Thread", "Doing function mafunc", GTG_PINK);

}


/* This function is called by eztrace_convert for each event to
 * handle.
 * It shall return 1 if the event was handled successfully or
 * 0 otherwise.
 */
int
handle_staticlib_events(struct fxt_ev_64 *ev)
{
  switch(ev->code)
    {
	case FUT_staticlib_mafunc_1:
	handleFUT_staticlib_mafunc_1();
        break;
	case FUT_staticlib_mafunc_2:
	handleFUT_staticlib_mafunc_2();
        break;

    default:
      /* The event was not handled */
      return 0;
    }
  return 1;
}

void handleFUT_staticlib_mafunc_1() {
    FUNC_NAME;
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    CHANGE() pushState (CURRENT, "ST_Thread", thread_id, "staticlib_STATE_0");
}
void handleFUT_staticlib_mafunc_2() {
    FUNC_NAME;
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    CHANGE() popState (CURRENT, "ST_Thread", thread_id);
}

