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

static void MPI_Bcast_prolog(void *buffer __attribute__((unused)),
			     int count,
			     MPI_Datatype datatype,
			     int root,
			     MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);
  int msize;
  MPI_Type_size(datatype, &msize);
  EZTRACE_EVENT5(FUT_MPI_START_BCast, comm, size, rank, msize*count, root);
}

static int MPI_Bcast_core(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  return libMPI_Bcast(buffer, count, datatype, root, comm);
}

static void MPI_Bcast_epilog(void *buffer __attribute__((unused)),
			     int count __attribute__((unused)),
			     MPI_Datatype datatype,
			     int root,
			     MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int msize;
  MPI_Type_size(datatype, &msize);
  EZTRACE_EVENT4(FUT_MPI_STOP_BCast, comm, size, rank, msize*count);
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
  FUNCTION_ENTRY;
  MPI_Bcast_prolog(buffer, count, datatype, root, comm);
  int ret = MPI_Bcast_core(buffer, count, datatype, root, comm);
  MPI_Bcast_epilog(buffer, count, datatype, root, comm);
  return ret;
}

void mpif_bcast_(void *buf, int *count, MPI_Fint *d,
		 int *root, MPI_Fint *c, int *error)
{
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Bcast_prolog(buf, *count, c_type, *root, c_comm);
  *error = MPI_Bcast_core(buf, *count, c_type, *root, c_comm);
  MPI_Bcast_epilog(buf, *count, c_type, *root, c_comm);
}
