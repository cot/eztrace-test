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

/* todo: implement this function ! */

static void MPI_Waitsome_prolog (int incount,
				 void *reqs,
				 int *outcount __attribute__((unused)),
				 int *array_of_indices __attribute__((unused)),
				 MPI_Status *array_of_statuses __attribute__((unused)),
				 size_t size )
{
  int i;
  for(i=0; i< incount; i++) {
    /* we can't use &reqs[i] here since req may be an array of MPI_Request or MPI_Fint
     * (which may have different size)
     */
    EZTRACE_EVENT1(FUT_MPI_START_WAIT, (void*)reqs + (i*size));
  }
}


static int MPI_Waitsome_core (int incount,
			      MPI_Request *reqs,
			      int *outcount,
			      int *array_of_indices,
			      MPI_Status *array_of_statuses )
{
  return libMPI_Waitsome(incount, reqs, outcount, array_of_indices, array_of_statuses);
}

static void MPI_Waitsome_epilog (int incount __attribute__((unused)),
				 void *reqs,
				 int *outcount,
				 int *array_of_indices,
				 MPI_Status *array_of_statuses __attribute__((unused)),
				 size_t size )
{
  int i;
  /* create a MPI_STOP_WAIT event for all successfull requests */
  for(i=0; i<*outcount; i++) {
    /* we can't use &reqs[i] here since req may be an array of MPI_Request or MPI_Fint
     * (which may have different size)
     */
    EZTRACE_EVENT1(FUT_MPI_STOP_WAIT, (void*) reqs + (array_of_indices[i]*size));
  }

}



int MPI_Waitsome (int incount,
		  MPI_Request *reqs,
		  int *outcount,
		  int *array_of_indices,
		  MPI_Status *array_of_statuses )
{
  FUNCTION_ENTRY;
  MPI_Waitsome_prolog(incount, reqs, outcount, array_of_indices, array_of_statuses, sizeof(MPI_Request));
  int ret = MPI_Waitsome_core(incount, reqs, outcount, array_of_indices, array_of_statuses);
  MPI_Waitsome_epilog(incount, reqs, outcount, array_of_indices, array_of_statuses, sizeof(MPI_Request));

  return ret;
}


void mpif_waitsome_(int *ic, MPI_Fint *r, int* oc, int* indexes, MPI_Status *s, int *error)
{
  int i;
  MPI_Waitsome_prolog(*ic, r, oc, indexes, s, sizeof(MPI_Fint));

  ALLOCATE_ITEMS(MPI_Request, *ic, c_req, p_req);

  for(i=0; i<*ic;i++)
    p_req[i] = MPI_Request_f2c(r[i]);

  *error = MPI_Waitsome_core(*ic, p_req, oc, indexes, s);

  for(i=0; i<*ic;i++)
    r[i]= MPI_Request_c2f(p_req[i]);

  MPI_Waitsome_epilog(*ic, r, oc, indexes, s, sizeof(MPI_Fint));
  FREE_ITEMS(*ic, p_req);
}
