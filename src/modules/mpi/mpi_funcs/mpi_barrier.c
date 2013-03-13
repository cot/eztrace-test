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

static void MPI_Barrier_prolog (MPI_Comm c)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(c, &size);
  libMPI_Comm_rank(c, &rank);
  EZTRACE_EVENT3 (FUT_MPI_START_BARRIER, c, rank, size);
}

static int MPI_Barrier_core (MPI_Comm c)
{
  return libMPI_Barrier(c);
}

static void MPI_Barrier_epilog (MPI_Comm c)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(c, &size);
  libMPI_Comm_rank(c, &rank);
  EZTRACE_EVENT3 (FUT_MPI_STOP_BARRIER, c, rank, size);
}

int MPI_Barrier (MPI_Comm c)
{
  FUNCTION_ENTRY;

  MPI_Barrier_prolog(c);
  int ret = MPI_Barrier_core(c);
  MPI_Barrier_epilog(c);

  return ret;
}

void mpif_barrier_(MPI_Fint *c, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Barrier_prolog(c_comm);
  *error = MPI_Barrier_core(c_comm);
  MPI_Barrier_epilog(c_comm);
}
