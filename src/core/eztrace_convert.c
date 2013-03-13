/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright � CNRS, INRIA, Universit� Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>

#include "ev_codes.h"
#include "eztrace_list.h"
#include "eztrace_convert.h"
#include "eztrace_convert_core.h"
#include "eztrace_hook.h"

#include "GTG.h"

/* name of the output file */
static char *output_filename = "paje";

static void
usage (int argc  __attribute__((unused)), char **argv)
{
  fprintf (stderr,
	   "Usage : %s [-v] [-t PAJE|OTF] [-z] [-o output_filename] input_filename input_filename ... \n",
	   argv[0]);
}

int compress = 0;

static void
parse_args (int argc, char **argv)
{
  int i;
  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-o") == 0)
	output_filename = argv[++i];
      else if (strcmp (argv[i], "-v") == 0)
	VERBOSE = 1;
      else if (strcmp (argv[i], "-z") == 0)
             compress = 1;
      else if (strcmp (argv[i], "-t") == 0){
	char* trace_type = argv[++i];
	if(strcmp(trace_type, "PAJE") == 0) {
	  printf("Setting trace type to PAJE\n");
	  setTraceType (PAJE);
	} else if(strcmp(trace_type, "OTF") == 0) {
	  printf("Setting trace type to OTF\n");
	  setTraceType (OTF);
	} else {
	  fprintf(stderr, "Unknown trace type: '%s'\n", trace_type);
	  exit(-1);
	}
      }
      else if(strcmp (argv[i], "-h") == 0) {
	usage (argc, argv);
	exit (-1);
      } else if ( argv[i][0] == '-' ) {
        fprintf(stderr, "Unknown option %s\n", argv[i]);
	usage (argc, argv);
	exit (-1);
      } else {
	eztrace_convert_init(argc-i);
	get_traces(NB_TRACES)->input_filename = argv[i];
	NB_TRACES++;
      }
    }

  if(NB_TRACES < 1)
    {
      usage (argc, argv);
      exit (-1);
    }
}

/*
 * This program should be used to parse the log file generated by FxT
 */
int
main (int argc, char **argv)
{
  int ret;
  int fd_in;

  load_modules(1);

  setTraceType (PAJE);

  /* parse the arguments passed to this program */
  parse_args (argc, argv);

#ifdef GTG_OUT_OF_ORDER
  ret = initTrace(output_filename, 0, GTG_FLAG_OUTOFORDER);
#else
  ret = initTrace(output_filename, 0, GTG_FLAG_NONE);
#endif
  if(ret != TRACE_SUCCESS) {
    fprintf(stderr, "fail to initialize GTG\n");
    return 1;
  }

  if(compress)
    if(setCompress(9) != TRACE_SUCCESS)
      fprintf(stderr, "Fail to enable trace compression\n");

  eztrace_initialize_gtg();

  __init_modules();


  int i;
  /* initialize the traces array */
  for(i=0;i< NB_TRACES; i++) {
    /* open the trace file */
    fxt_t fut = fxt_open (get_traces(i)->input_filename);
    if (!fut)
      {
	perror ("fxt_open:");
	exit (-1);
      }

    get_traces(i)->delay = 0;
    get_traces(i)->rank = i;
    get_traces(i)->id = i;
    get_traces(i)->done = 0;
    get_traces(i)->skip = 0;
    get_traces(i)->line_number = 0;

    eztrace_create_containers(i);

    /* if several traces are loaded, this means that MPI was used,
     * so let's skip all the first events until MPI_Init is detected
     */
    if(NB_TRACES > 1) {
      get_traces(i)->start = 0;
      get_traces(i)->trace_id = NULL;
    } else {
      CREATE_TRACE_ID_STR(get_traces(i)->trace_id, 0);
      get_traces(i)->start = 1;
      NB_START= 1;
      addContainer (0.00000, get_traces(i)->trace_id, "CT_Process", "C_Prog", get_traces(i)->trace_id, "0");
      eztrace_create_ids(get_traces(i)->rank);
    }

    get_traces(i)->block = fxt_blockev_enter (fut);

    ret = fxt_next_ev (get_traces(i)->block, FXT_EV_TYPE_64, (struct fxt_ev *) &get_traces(i)->ev);
    if (ret != FXT_EV_OK)
      {
	fprintf (stderr, "no more block ...\n");
	break;
      }

    get_traces(i)->start_time = get_traces(i)->ev.time;
  }

  /* todo: 0 or i ? */
  set_cur_trace(get_traces(0));
  set_cur_ev(&get_traces(i)->ev);

  struct eztrace_event_handler* h_info = NULL;
  h_info = get_handler_info();

  h_info->cur_trace_nb = 0;
  h_info->nb_done = 0;
  h_info->nb_handled = 0;
  sem_init(&h_info->events_processed, 0, 0);

  /* create the handler thread and wait until it completes */
  create_main_thread();
  wake_up_handler_thread();
  sem_wait(&h_info->events_processed);

  /* finalize the trace and close the file */
  endTrace ();
  eztrace_convert_finalize();
  printf("%d events handled\n", h_info->nb_handled);
  return 0;
}