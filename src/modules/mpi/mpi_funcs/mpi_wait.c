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


static void MPI_Wait_prolog(MPI_Fint *req, MPI_Status *s __attribute__((unused)))
{
  EZTRACE_EVENT1(FUT_MPI_START_WAIT, req);
}

static int MPI_Wait_core(MPI_Request *req, MPI_Status *s)
{
  return libMPI_Wait(req, s);
}

static void MPI_Wait_epilog(MPI_Fint *req, MPI_Status *s __attribute__((unused)))
{
  EZTRACE_EVENT1(FUT_MPI_STOP_WAIT, req);
}


int MPI_Wait(MPI_Request *req, MPI_Status *s)
{
  FUNCTION_ENTRY;
  MPI_Wait_prolog((MPI_Fint*)req, s);
  int ret = MPI_Wait_core(req, s);
  MPI_Wait_epilog((MPI_Fint*) req, s);
  return ret;
}

void mpif_wait_(MPI_Fint *r, MPI_Fint *s, int *error)
{
  MPI_Request c_req = MPI_Request_f2c(*r);
  MPI_Status c_status;
  MPI_Wait_prolog(r, &c_status);
  *error = MPI_Wait_core(&c_req, &c_status);
  MPI_Status_c2f(&c_status, s);
  MPI_Wait_epilog(r, &c_status);
}

