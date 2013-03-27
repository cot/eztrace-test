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


static void MPI_Alltoall_prolog (CONST void *sendbuf __attribute__((unused)),
				 int sendcount __attribute__((unused)),
				 MPI_Datatype sendtype,
				 void *recvbuf __attribute__((unused)),
				 int recvcnt __attribute__((unused)),
				 MPI_Datatype recvtype,
				 MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT3(FUT_MPI_START_Alltoall, comm, size, rank);

}

static int MPI_Alltoall_core (CONST void *sendbuf, int sendcount, MPI_Datatype sendtype,
			      void *recvbuf, int recvcnt, MPI_Datatype recvtype,
			      MPI_Comm comm)
{
  return libMPI_Alltoall (sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm);
}

static void MPI_Alltoall_epilog (CONST void *sendbuf __attribute__((unused)),
				 int sendcount __attribute__((unused)),
				 MPI_Datatype sendtype,
				 void *recvbuf __attribute__((unused)),
				 int recvcnt __attribute__((unused)),
				 MPI_Datatype recvtype,
				 MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT3(FUT_MPI_STOP_Alltoall, comm, size, rank);
}


int MPI_Alltoall (CONST void *sendbuf, int sendcount, MPI_Datatype sendtype,
                  void *recvbuf, int recvcnt, MPI_Datatype recvtype,
		  MPI_Comm comm)
{
	int ret;
	FUNCTION_ENTRY;
	if(_UNUSED_MPI)
		ret = MPI_Alltoall_core (sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm);
	else
	{
		MPI_Alltoall_prolog (sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm);
		int ret = MPI_Alltoall_core (sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm);
		MPI_Alltoall_epilog (sendbuf, sendcount, sendtype, recvbuf, recvcnt, recvtype, comm);
	}
  return ret;
}


void mpif_alltoall_(void *sbuf, int *scount, MPI_Fint *sd,
		    void *rbuf, int *rcount, MPI_Fint *rd,
		    MPI_Fint *c, int *error)
{
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Alltoall_prolog(sbuf, *scount, c_stype, rbuf, *rcount, c_rtype, c_comm);
  *error = MPI_Alltoall_core(sbuf, *scount, c_stype, rbuf, *rcount, c_rtype, c_comm);
  MPI_Alltoall_epilog(sbuf, *scount, c_stype, rbuf, *rcount, c_rtype, c_comm);
}
