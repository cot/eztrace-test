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



static void MPI_Startall_prolog(int count, void*req, size_t size)
{
 int i;
 for(i=0; i<count; i++)
    /* we can't use &req[i] here since req may be an array of MPI_Request or MPI_Fint
     * (which may have different size)
     */
   EZTRACE_EVENT1(FUT_MPI_START, (void*)req + (i*size));
}

static int MPI_Startall_core(int count, MPI_Request *req)
{
  return libMPI_Startall(count, req);

}

int MPI_Startall(int count, MPI_Request *req)
{
  FUNCTION_ENTRY;

  MPI_Startall_prolog(count, req, sizeof(MPI_Request));
  int ret = MPI_Startall_core(count, req);

  return ret;
}


void mpif_startall_(int*count, MPI_Fint *reqs, int*error)
{
  int i;
  ALLOCATE_ITEMS(MPI_Request, *count, c_req, p_req);

  for(i=0; i<*count;i++)
    p_req[i] = MPI_Request_f2c(reqs[i]);

  MPI_Startall_prolog(*count, reqs, sizeof(MPI_Fint));
  *error = MPI_Startall_core(*count, p_req);

  for(i=0; i<*count;i++)
    reqs[i]= MPI_Request_c2f(p_req[i]);

  FREE_ITEMS(*count, p_req);
}
