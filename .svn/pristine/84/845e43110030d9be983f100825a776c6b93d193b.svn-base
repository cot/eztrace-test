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


static int MPI_Test_core(MPI_Request *req, int *a, MPI_Status *s)
{
  return libMPI_Test(req, a, s);
}

static void MPI_Test_epilog(MPI_Fint *req, int *a, MPI_Status *s __attribute__((unused)))
{
  if (*a)
    EZTRACE_EVENT1(FUT_MPI_TEST_SUCCESS, req);
}


int MPI_Test(MPI_Request *req, int *a, MPI_Status *s)
{
  FUNCTION_ENTRY;
  int res = MPI_Test_core(req, a, s);
  MPI_Test_epilog((MPI_Fint*)req, a, s);
  return res;
}

void mpif_test_(MPI_Fint *r, int *f, MPI_Fint *s, int *error)
{
  MPI_Request c_req = MPI_Request_f2c(*r);
  MPI_Status c_status;

  *error = MPI_Test_core(&c_req, f, &c_status);
  *r = MPI_Request_c2f(c_req);

  if(*f) {
    MPI_Status_c2f(&c_status, s);
  }

  MPI_Test_epilog(r, f, &c_status);
}
