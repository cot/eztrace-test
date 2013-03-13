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


static void MPI_Probe_prolog( int source __attribute__((unused)),
			      int tag __attribute__((unused)),
			      MPI_Comm comm __attribute__((unused)),
			      MPI_Status *status __attribute__((unused)) )
{
  EZTRACE_EVENT0(FUT_MPI_START_PROBE);
}

static int MPI_Probe_core( int source, int tag, MPI_Comm comm, MPI_Status *status )
{
  return libMPI_Probe(source, tag, comm, status);
}

static void MPI_Probe_epilog( int source __attribute__((unused)),
			      int tag __attribute__((unused)),
			      MPI_Comm comm __attribute__((unused)),
			      MPI_Status *status )
{
  int length = -1;
  MPI_Get_count(status, MPI_BYTE, &length);
  EZTRACE_EVENT3(FUT_MPI_STOP_PROBE, status->MPI_SOURCE, status->MPI_TAG, length);
}


int MPI_Probe( int source, int tag, MPI_Comm comm, MPI_Status *status )
{
  FUNCTION_ENTRY;
  MPI_Probe_prolog(source, tag, comm, status);
  int ret = MPI_Probe_core(source, tag, comm, status);
  MPI_Probe_epilog(source, tag, comm, status);
  return ret;
}

void mpif_probe_( int* source, int* tag, MPI_Fint* comm, MPI_Status *status, int* err )
{
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Probe_prolog(*source, *tag, c_comm, status);
  *err = MPI_Probe_core(*source, *tag, c_comm, status);
  MPI_Probe_epilog(*source, *tag, c_comm, status);
}
