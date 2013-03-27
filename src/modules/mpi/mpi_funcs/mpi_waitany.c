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

static void MPI_Waitany_prolog (int count, void*reqs,
				int *index __attribute__((unused)),
				MPI_Status *status __attribute__((unused)),
				size_t size)
{
  EZTRACE_EVENT1(FUT_MPI_START_WAITANY, count);
  int i;
  for(i=0;i<count; i++) {
    /* we can't use &req[i] here since req may be an array of MPI_Request or MPI_Fint
     * (which may have different size)
     */
    EZTRACE_EVENT1(FUT_MPI_Info, (void*)reqs+(i*size));
  }
}

static int MPI_Waitany_core (int count, MPI_Request *reqs, int *index, MPI_Status *status)
{
  return libMPI_Waitany(count, reqs, index, status);
}

static void MPI_Waitany_epilog (int count,
				void *reqs,
				int *index,
				MPI_Status *status __attribute__((unused)),
				size_t size)
{
  int i;
  EZTRACE_EVENT2(FUT_MPI_STOP_WAITANY, count, *index);

  for(i=0;i<count; i++) {
    /* we can't use &req[i] here since req may be an array of MPI_Request or MPI_Fint
     * (which may have different size)
     */
    EZTRACE_EVENT1(FUT_MPI_Info, (void*)reqs+(i*size));
  }
}

int MPI_Waitany (int count, MPI_Request *reqs, int *index, MPI_Status *status)
{
	int ret;
	FUNCTION_ENTRY;
	if(_UNUSED_MPI)
		ret = MPI_Waitany_core(count, reqs, index, status);
	else
	{
		MPI_Waitany_prolog(count, reqs, index, status, sizeof(MPI_Request));
		ret = MPI_Waitany_core(count, reqs, index, status);
		MPI_Waitany_epilog(count, reqs, index, status, sizeof(MPI_Request));
	}
	return ret;
}



void mpif_waitany_(int *c, MPI_Fint *r, MPI_Status *s, int *index, int *error)
{
  int i;
  MPI_Waitany_prolog(*c, r, index, s, sizeof(MPI_Fint));

  ALLOCATE_ITEMS(MPI_Request, *c, c_req, p_req);

  for(i=0; i<*c;i++)
    p_req[i] = MPI_Request_f2c(r[i]);
  *error = MPI_Waitany_core(*c, p_req, index, s);
  for(i=0; i<*c;i++)
    r[i]= MPI_Request_c2f(p_req[i]);

  MPI_Waitany_epilog(*c, r, index, s, sizeof(MPI_Fint));
  FREE_ITEMS(*c, p_req);
}
