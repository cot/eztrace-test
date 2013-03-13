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



static void MPI_Alltoallv_prolog (CONST void *sendbuf __attribute__((unused)),
				  CONST int *sendcnts __attribute__((unused)),
				  CONST int *sdispls __attribute__((unused)),
				  MPI_Datatype sendtype,
				  void *recvbuf __attribute__((unused)),
				  CONST int *recvcnts __attribute__((unused)),
				  CONST int *rdispls __attribute__((unused)),
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

  EZTRACE_EVENT3(FUT_MPI_START_Alltoallv, comm, size, rank);
}

static int MPI_Alltoallv_core (CONST void *sendbuf, CONST int *sendcnts, CONST int *sdispls, MPI_Datatype sendtype,
			       void *recvbuf, CONST int *recvcnts, CONST int *rdispls, MPI_Datatype recvtype,
			       MPI_Comm comm)
{
  return libMPI_Alltoallv (sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm);
}

static void MPI_Alltoallv_epilog (CONST void *sendbuf __attribute__((unused)),
				  CONST int *sendcnts __attribute__((unused)),
				  CONST int *sdispls __attribute__((unused)),
				  MPI_Datatype sendtype,
				  void *recvbuf __attribute__((unused)),
				  CONST int *recvcnts __attribute__((unused)),
				  CONST int *rdispls __attribute__((unused)),
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

  EZTRACE_EVENT3(FUT_MPI_STOP_Alltoallv, comm, size, rank);
}


int MPI_Alltoallv (CONST void *sendbuf, CONST int *sendcnts, CONST int *sdispls, MPI_Datatype sendtype,
		   void *recvbuf, CONST int *recvcnts, CONST int *rdispls, MPI_Datatype recvtype,
		   MPI_Comm comm)
{
  FUNCTION_ENTRY;

  MPI_Alltoallv_prolog (sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm);
  int ret = MPI_Alltoallv_core (sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm);
  MPI_Alltoallv_epilog (sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm);

  return ret;
}

void mpif_alltoallv_(void *sbuf, int *scount, int *sdispls, MPI_Fint *sd,
		     void *rbuf, int *rcount, int *rdispls, MPI_Fint *rd,
		     MPI_Fint *c, int *error)
{
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Alltoallv_prolog(sbuf, scount, sdispls, c_stype, rbuf, rcount, rdispls, c_rtype, c_comm);
  *error = MPI_Alltoallv_core(sbuf, scount, sdispls, c_stype, rbuf, rcount, rdispls, c_rtype, c_comm);
  MPI_Alltoallv_epilog(sbuf, scount, sdispls, c_stype, rbuf, rcount, rdispls, c_rtype, c_comm);
}
