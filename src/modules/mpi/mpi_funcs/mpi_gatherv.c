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

static void MPI_Gatherv_prolog(CONST void *sendbuf __attribute__((unused)),
			       int sendcnt,
			       MPI_Datatype sendtype,
			       void *recvbuf __attribute__((unused)),
			       CONST int *recvcnts __attribute__((unused)),
			       CONST int *displs __attribute__((unused)),
			       MPI_Datatype recvtype,
			       int root,
			       MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT5(FUT_MPI_START_Gatherv, comm, size, rank, ssize*sendcnt, root);

}

static int MPI_Gatherv_core(CONST void *sendbuf, int sendcnt, MPI_Datatype sendtype,
			    void *recvbuf, CONST int *recvcnts, CONST int *displs,
			    MPI_Datatype recvtype, int root, MPI_Comm comm)
{
  return libMPI_Gatherv(sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm);
}

static void MPI_Gatherv_epilog(CONST void *sendbuf __attribute__((unused)),
			       int sendcnt __attribute__((unused)),
			       MPI_Datatype sendtype,
			       void *recvbuf __attribute__((unused)),
			       CONST int *recvcnts __attribute__((unused)),
			       CONST int *displs __attribute__((unused)),
			       MPI_Datatype recvtype,
			       int root __attribute__((unused)),
			       MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT3(FUT_MPI_STOP_Gatherv, comm, size, rank);
}

int MPI_Gatherv(CONST void *sendbuf, int sendcnt, MPI_Datatype sendtype,
		void *recvbuf, CONST int *recvcnts, CONST int *displs,
		MPI_Datatype recvtype, int root, MPI_Comm comm)
{
  FUNCTION_ENTRY;

  MPI_Gatherv_prolog(sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm);
  int ret = MPI_Gatherv_core(sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm);
  MPI_Gatherv_epilog(sendbuf, sendcnt, sendtype, recvbuf, recvcnts, displs, recvtype, root, comm);

  return ret;
}


void mpif_gatherv_(void *sbuf, int *scount, MPI_Fint *sd,
		   void *rbuf, int *rcount, int *displs,
		   MPI_Fint *rd, int *root, MPI_Fint *c, int *error)
{
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Gatherv_prolog(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, *root, c_comm);
  *error = MPI_Gatherv_core(sbuf, *scount, c_stype, rbuf, rcount,
			    displs, c_rtype, *root, c_comm);
  MPI_Gatherv_epilog(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, *root, c_comm);
}
