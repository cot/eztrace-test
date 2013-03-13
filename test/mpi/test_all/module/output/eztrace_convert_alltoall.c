#include <stdio.h>
#include <GTG.h>
#include "eztrace_convert.h"
#include "alltoall_ev_codes.h"

int eztrace_convert_alltoall_init();

int handle_alltoall_events(struct fxt_ev_64 *ev);

void handleFUT_alltoall_MPI_Alltoall_1() ;
void handleFUT_alltoall_MPI_Alltoall_2() ;




/* Constructor of the plugin.
 * This function registers the current module to eztrace_convert
 */
struct eztrace_convert_module alltoall_module;
void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  alltoall_module.api_version = EZTRACE_API_VERSION;

  /* Specify the initialization function.
   * This function will be called once all the plugins are loaded
   * and the trace is started.
   * This function usually declared StateTypes, LinkTypes, etc.
   */
  alltoall_module.init = eztrace_convert_alltoall_init;

  /* Specify the function to call for handling an event
   */
  alltoall_module.handle = handle_alltoall_events;

  /* Specify the module prefix */
  alltoall_module.module_prefix = alltoall_EVENTS_ID;

  asprintf(&alltoall_module.name, "alltoall");
  asprintf(&alltoall_module.description, "module for the alltoall mpi library");

  alltoall_module.token.data = &alltoall_module;

  /* Register the module to eztrace_convert */
  eztrace_convert_register_module(&alltoall_module);

  //printf("module alltoall loaded\n");
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
  //printf("unloading module alltoall\n");
}



/*
 * This function will be called once all the plugins are loaded
 * and the trace is started.
 * This function usually declared StateTypes, LinkTypes, etc.
 */
int
eztrace_convert_alltoall_init()
{
    addEntityValue("alltoall_STATE_0", "ST_Thread", "Doing function MPI_Alltoall", GTG_PINK);

}


/* This function is called by eztrace_convert for each event to
 * handle.
 * It shall return 1 if the event was handled successfully or
 * 0 otherwise.
 */
int
handle_alltoall_events(struct fxt_ev_64 *ev)
{
  switch(ev->code)
    {
	case FUT_alltoall_MPI_Alltoall_1:
	handleFUT_alltoall_MPI_Alltoall_1();
        break;
	case FUT_alltoall_MPI_Alltoall_2:
	handleFUT_alltoall_MPI_Alltoall_2();
        break;

    default:
      /* The event was not handled */
      return 0;
    }
  return 1;
}

void handleFUT_alltoall_MPI_Alltoall_1() {
    FUNC_NAME;
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    CHANGE() pushState (CURRENT, "ST_Thread", thread_id, "alltoall_STATE_0");
}
void handleFUT_alltoall_MPI_Alltoall_2() {
    FUNC_NAME;
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
    CHANGE() popState (CURRENT, "ST_Thread", thread_id);
}

