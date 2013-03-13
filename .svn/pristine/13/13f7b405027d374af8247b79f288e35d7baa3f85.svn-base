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


static void MPI_Start_prolog(MPI_Fint *req)
{
  EZTRACE_EVENT1(FUT_MPI_START, req);
}

static int MPI_Start_core(MPI_Request *req)
{
  return libMPI_Start(req);
}


int MPI_Start(MPI_Request *req)
{
  FUNCTION_ENTRY;

  MPI_Start_prolog((MPI_Fint*) req);
  int ret = MPI_Start_core(req);

  return ret;
}

void mpif_start_(MPI_Fint *req, int*error)
{
  MPI_Request c_req = MPI_Request_f2c(*req);

  MPI_Start_prolog(req);
  *error = MPI_Start_core(&c_req);

  *req = MPI_Request_c2f(c_req);
}
