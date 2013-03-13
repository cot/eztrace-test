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


static int MPI_Testall_core (int count, MPI_Request *reqs, int *flag, MPI_Status *s)
{
  return libMPI_Testall(count, reqs, flag, s);
}

static void MPI_Testall_epilog (int count,
				MPI_Fint *reqs,
				int *flag,
				MPI_Status *s __attribute__((unused)),
				size_t size)
{
  if(*flag) {
    int i;
    for(i=0; i<count; i++)
      EZTRACE_EVENT1(FUT_MPI_TEST_SUCCESS, (void*)reqs + (i*size));
  }
}

int MPI_Testall (int count, MPI_Request *reqs, int *flag, MPI_Status *s)
{
  FUNCTION_ENTRY;
  int ret = MPI_Testall_core(count, reqs, flag, s);
  MPI_Testall_epilog(count, reqs, flag, s, sizeof(MPI_Request));

  return ret;
}


void mpif_testall_(int *count, MPI_Fint *r, int *index, MPI_Status *s, int *error)
{
  int i;
  ALLOCATE_ITEMS(MPI_Request, *count, c_req, p_req);

  for(i=0; i<*count;i++)
    p_req[i] = MPI_Request_f2c(r[i]);
  *error = MPI_Testall_core(*count, p_req, index, s);
  for(i=0; i<*count;i++)
    r[i]= MPI_Request_c2f(p_req[i]);

  MPI_Testall_epilog(*count, r, index, s, sizeof(MPI_Fint));
  FREE_ITEMS(*count, p_req);
}
