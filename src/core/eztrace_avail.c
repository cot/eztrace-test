/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "eztrace_convert_core.h"

/* This program list the available modules
 */
static void
usage (int argc  __attribute__((unused)), char **argv)
{
  fprintf (stderr,
	   "Usage : %s -h \n",
	   argv[0]);
}

static void
parse_args (int argc, char **argv)
{
  int i;
  for (i = 1; i < argc; i++)
    {
      if(strcmp (argv[i], "-h") == 0) {
	usage (argc, argv);
	exit (-1);
      } else if (strcmp (argv[i], "-v") == 0) {
	printf("Verbose mode turned on\n");
	(*get_verbose()) = 1;
      }
    }
}

int main(int argc, char**argv)
{
  /* clear the EZTRACE_TRACE environment variable so that all the available modules are listed */
  unsetenv("EZTRACE_TRACE");

  /* parse the arguments passed to this program */
  parse_args (argc, argv);

  load_modules(0);
  eztrace_convert_list();

  return 0;
}
