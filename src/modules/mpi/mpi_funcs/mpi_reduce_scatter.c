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


static void MPI_Reduce_scatter_prolog (CONST void *sendbuf __attribute__((unused)),
				       void *recvbuf __attribute__((unused)),
				       CONST int *recvcnts __attribute__((unused)),
				       MPI_Datatype datatype,
				       MPI_Op op __attribute__((unused)),
				       MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int len;
  MPI_Type_size(datatype, &len);

  EZTRACE_EVENT3(FUT_MPI_START_Reduce_scatter, comm, size, rank);
}

static int MPI_Reduce_scatter_core (CONST void *sendbuf, void *recvbuf, CONST int *recvcnts,
				    MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  return libMPI_Reduce_scatter (sendbuf, recvbuf, recvcnts, datatype, op, comm);
}

static void MPI_Reduce_scatter_epilog (CONST void *sendbuf __attribute__((unused)),
				       void *recvbuf __attribute__((unused)),
				       CONST int *recvcnts __attribute__((unused)),
				       MPI_Datatype datatype,
				       MPI_Op op __attribute__((unused)),
				       MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int len;
  MPI_Type_size(datatype, &len);

  EZTRACE_EVENT3(FUT_MPI_STOP_Reduce_scatter, comm, size, rank);
}


int MPI_Reduce_scatter (CONST void *sendbuf, void *recvbuf, CONST int *recvcnts,
			MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  FUNCTION_ENTRY;

  MPI_Reduce_scatter_prolog (sendbuf, recvbuf, recvcnts, datatype, op, comm);
  int ret = MPI_Reduce_scatter_core (sendbuf, recvbuf, recvcnts, datatype, op, comm);
  MPI_Reduce_scatter_epilog (sendbuf, recvbuf, recvcnts, datatype, op, comm);
  return ret;
}

void mpif_reduce_scatter_(void *sbuf, void *rbuf, int *rcount,
			  MPI_Fint *d, MPI_Fint *op,
			  MPI_Fint *c, int *error)
{
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Op c_op = MPI_Op_f2c(*op);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Reduce_scatter_prolog(sbuf, rbuf, rcount, c_type, c_op, c_comm);
  *error = MPI_Reduce_scatter_core(sbuf, rbuf, rcount, c_type, c_op, c_comm);
  MPI_Reduce_scatter_epilog(sbuf, rbuf, rcount, c_type, c_op, c_comm);
}
