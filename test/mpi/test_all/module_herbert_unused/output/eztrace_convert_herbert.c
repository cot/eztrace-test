#include <stdio.h>
#include <GTG.h>
#include "eztrace_convert.h"
#include "herbert_ev_codes.h"

int eztrace_convert_herbert_init();

int handle_herbert_events(struct fxt_ev_64 *ev);

void handleFUT_herbert_herbertvonkrugell_1() ;
void handleFUT_herbert_herbertvonkrugell_2() ;




/* Constructor of the plugin.
 * This function registers the current module to eztrace_convert
 */
struct eztrace_convert_module herbert_module;
void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  herbert_module.api_version = EZTRACE_API_VERSION;

  /* Specify the initialization function.
   * This function will be called once all the plugins are loaded
   * and the trace is started.
   * This function usually declared StateTypes, LinkTypes, etc.
   */
  herbert_module.init = eztrace_convert_herbert_init;

  /* Specify the function to call for handling an event
   */
  herbert_module.handle = handle_herbert_events;

  /* Specify the module prefix */
  herbert_module.module_prefix = herbert_EVENTS_ID;

  asprintf(&herbert_module.name, "herbert");
  asprintf(&herbert_module.description, "module for herbert");

  herbert_module.token.data = &herbert_module;

  /* Register the module to eztrace_convert */
  eztrace_convert_register_module(&herbert_module);

  //printf("module herbert loaded\n");
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
  //printf("unloading module herbert\n");
}



/*
 * This function will be called once all the plugins are loaded
 * and the trace is started.
 * This function usually declared StateTypes, LinkTypes, etc.
 */
int
eztrace_convert_herbert_init()
{
    addEntityValue("herbert_STATE_0", "ST_Thread", "Doing function herbertvonkrugell", GTG_PINK);

}


/* This function is called by eztrace_convert for each event to
 * handle.
 * It shall return 1 if the event was handled successfully or
 * 0 otherwise.
 */
int
handle_herbert_events(struct fxt_ev_64 *ev)
{
  switch(ev->code)
    {
	case FUT_herbert_herbertvonkrugell_1:
	handleFUT_herbert_herbertvonkrugell_1();
        break;
	case FUT_herbert_herbertvonkrugell_2:
	handleFUT_herbert_herbertvonkrugell_2();
        break;

    default:
      /* The event was not handled */
      return 0;
    }
  return 1;
}

void handleFUT_herbert_herbertvonkrugell_1() {
    FUNC_NAME;
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    CHANGE() pushState (CURRENT, "ST_Thread", thread_id, "herbert_STATE_0");
}
void handleFUT_herbert_herbertvonkrugell_2() {
    FUNC_NAME;
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    CHANGE() popState (CURRENT, "ST_Thread", thread_id);
}

