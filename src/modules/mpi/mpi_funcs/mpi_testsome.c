/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE 1
#define _REENTRANT

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <dlfcn.h>
#include <string.h>

#include "mpi.h"
#include "mpi_eztrace.h"
#include "mpi_ev_codes.h"
#include "eztrace.h"

static int MPI_Testsome_core (int incount,
			      MPI_Request *reqs,
			      int *outcount,
			      int *indexes,
			      MPI_Status *statuses)
{
  return libMPI_Testsome(incount, reqs, outcount, indexes, statuses);
}

static void MPI_Testsome_epilog (int incount __attribute__((unused)),
				 void *reqs,
				 int *outcount,
				 int *indexes __attribute__((unused)),
				 MPI_Status *statuses __attribute__((unused)),
				 size_t size)
{
  if (*outcount) {
    int i;
    for(i=0; i<*outcount; i++) {
      EZTRACE_EVENT1(FUT_MPI_TEST_SUCCESS, (void*)reqs + (i*size));
    }
  }
}

int MPI_Testsome (int incount,
		  MPI_Request *reqs,
		  int *outcount,
		  int *indexes,
		  MPI_Status *statuses)
{
  FUNCTION_ENTRY;
  int res = MPI_Testsome_core(incount, reqs, outcount, indexes, statuses);
  MPI_Testsome_epilog(incount, reqs, outcount, indexes, statuses, sizeof(MPI_Request));

  return res;
}



void mpif_testsome_(int *ic, MPI_Fint *r, int* oc, int* indexes, MPI_Status *s, int *error)
{
  int i;
  ALLOCATE_ITEMS(MPI_Request, *ic, c_req, p_req);

  for(i=0; i<*ic;i++)
    p_req[i] = MPI_Request_f2c(r[i]);

  *error = MPI_Testsome_core(*ic, p_req, oc, indexes, s);

  for(i=0; i<*ic;i++)
    r[i]= MPI_Request_c2f(p_req[i]);

  MPI_Testsome_epilog(*ic, r, oc, indexes, s, sizeof(MPI_Fint));

  FREE_ITEMS(*ic, p_req);
}
