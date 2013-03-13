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


static void MPI_Scatter_prolog(CONST void *sendbuf __attribute__((unused)),
			       int sendcnt,
			       MPI_Datatype sendtype,
			       void *recvbuf __attribute__((unused)),
			       int recvcnt __attribute__((unused)),
			       MPI_Datatype recvtype,
			       int root,
			       MPI_Comm comm )
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT5(FUT_MPI_START_Scatter, comm, size, rank, ssize*sendcnt, root);
}

static int MPI_Scatter_core(CONST void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcnt,
			    MPI_Datatype recvtype, int root, MPI_Comm comm )
{
  return libMPI_Scatter(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
}

static void MPI_Scatter_epilog(CONST void *sendbuf __attribute__((unused)),
			       int sendcnt __attribute__((unused)),
			       MPI_Datatype sendtype __attribute__((unused)),
			       void *recvbuf __attribute__((unused)),
			       int recvcnt __attribute__((unused)),
			       MPI_Datatype recvtype __attribute__((unused)),
			       int root __attribute__((unused)),
			       MPI_Comm comm )
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  EZTRACE_EVENT3(FUT_MPI_STOP_Scatter, comm, size, rank);

}

int MPI_Scatter(CONST void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcnt,
		MPI_Datatype recvtype, int root, MPI_Comm comm )
{
  FUNCTION_ENTRY;
  MPI_Scatter_prolog(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
  int ret = MPI_Scatter_core(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);
  MPI_Scatter_epilog(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm);

  return ret;
}


void mpif_scatter_(void *sbuf, int *scount, MPI_Fint *sd,
		   void *rbuf, int *rcount, MPI_Fint *rd,
		   int *root, MPI_Fint *c, int *error)
{
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Scatter_prolog(sbuf, *scount, c_stype,
		     rbuf, *rcount, c_rtype,
		     *root, c_comm);

  *error = MPI_Scatter_core(sbuf, *scount, c_stype,
			    rbuf, *rcount, c_rtype,
			    *root, c_comm);

  MPI_Scatter_epilog(sbuf, *scount, c_stype,
		     rbuf, *rcount, c_rtype,
		     *root, c_comm);
}
