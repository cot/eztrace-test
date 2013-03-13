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

static int MPI_Testany_core (int count, MPI_Request *reqs, int *index, int *flag, MPI_Status *status)
{
  return libMPI_Testany(count, reqs, index, flag, status);
}

static void MPI_Testany_epilog (int count __attribute__((unused)),
				void *reqs,
				int *index,
				int *flag,
				MPI_Status *status __attribute__((unused)),
				size_t size)
{
  if(*flag)
    EZTRACE_EVENT1(FUT_MPI_TEST_SUCCESS, (void*)reqs + ((*index)*size));
}


int MPI_Testany (int count, MPI_Request *reqs, int *index, int *flag, MPI_Status *status)
{
  FUNCTION_ENTRY;
  int ret = MPI_Testany_core(count, reqs, index, flag, status);
  MPI_Testany_epilog(count, reqs, index, flag, status, sizeof(MPI_Request));
  return ret;
}

void mpif_testany_(int *count, MPI_Fint *r, int *index, int *flag, MPI_Status *s, int *error)
{
  int i;
  ALLOCATE_ITEMS(MPI_Request, *count, c_req, p_req);

  for(i=0; i<*count;i++)
    p_req[i] = MPI_Request_f2c(r[i]);
  *error = MPI_Testany_core(*count, p_req, index, flag, s);
  for(i=0; i<*count;i++)
    r[i]= MPI_Request_c2f(p_req[i]);

  MPI_Testany_epilog(*count, r, index, flag, s, sizeof(MPI_Fint));
  FREE_ITEMS(*count, p_req);
}
