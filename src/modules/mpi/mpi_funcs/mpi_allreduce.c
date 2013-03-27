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


static void MPI_Allreduce_prolog (CONST void *sendbuf __attribute__((unused)),
				  void *recvbuf __attribute__((unused)),
				  int count __attribute__((unused)),
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

  EZTRACE_EVENT3(FUT_MPI_START_Allreduce, comm, size, rank);
}

static int MPI_Allreduce_core (CONST void *sendbuf, void *recvbuf, int count,
			       MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
  return libMPI_Allreduce (sendbuf, recvbuf, count, datatype, op, comm);
}

static void MPI_Allreduce_epilog (CONST void *sendbuf __attribute__((unused)),
				  void *recvbuf __attribute__((unused)),
				  int count __attribute__((unused)),
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

  EZTRACE_EVENT3(FUT_MPI_STOP_Allreduce, comm, size, rank);
}


int MPI_Allreduce (CONST void *sendbuf, void *recvbuf, int count,
                   MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
	int ret;
	FUNCTION_ENTRY;
	if(_UNUSED_MPI)
		ret = MPI_Allreduce_core (sendbuf, recvbuf, count, datatype, op, comm);
	else
	{
		MPI_Allreduce_prolog (sendbuf, recvbuf, count, datatype, op, comm);
		ret = MPI_Allreduce_core (sendbuf, recvbuf, count, datatype, op, comm);
		MPI_Allreduce_epilog (sendbuf, recvbuf, count, datatype, op, comm);
	}
  return ret;
}


void mpif_allreduce_(void *sbuf, void *rbuf, int *count,
		     MPI_Fint *d, MPI_Fint *op,
		     MPI_Fint *c, int *error)
{
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Op c_op = MPI_Op_f2c(*op);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Allreduce_prolog(sbuf, rbuf, *count, c_type, c_op, c_comm);
  *error = MPI_Allreduce_core(sbuf, rbuf, *count, c_type, c_op, c_comm);
  MPI_Allreduce_epilog(sbuf, rbuf, *count, c_type, c_op, c_comm);
}
