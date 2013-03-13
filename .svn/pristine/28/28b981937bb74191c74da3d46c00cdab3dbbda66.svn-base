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


static void MPI_Scatterv_prolog(CONST void *sendbuf __attribute__((unused)),
				CONST int *sendcnts __attribute__((unused)),
				CONST int *displs __attribute__((unused)),
				MPI_Datatype sendtype __attribute__((unused)),
				void *recvbuf __attribute__((unused)),
				int recvcnt,
				MPI_Datatype recvtype,
				int root,
				MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int rsize;
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT5(FUT_MPI_START_Scatterv, comm, size, rank, rsize*recvcnt, root);
}

static int MPI_Scatterv_core(CONST void *sendbuf,
			     CONST int *sendcnts,
			     CONST int *displs,
			     MPI_Datatype sendtype,
			     void *recvbuf,
			     int recvcnt,
			     MPI_Datatype recvtype,
			     int root,
			     MPI_Comm comm)
{
  return libMPI_Scatterv(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm);
}

static void MPI_Scatterv_epilog(CONST void *sendbuf __attribute__((unused)),
				CONST int *sendcnts __attribute__((unused)),
				CONST int *displs __attribute__((unused)),
				MPI_Datatype sendtype __attribute__((unused)),
				void *recvbuf __attribute__((unused)),
				int recvcnt __attribute__((unused)),
				MPI_Datatype recvtype __attribute__((unused)),
				int root __attribute__((unused)),
				MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  EZTRACE_EVENT3(FUT_MPI_STOP_Scatterv, comm, size, rank);

}


int MPI_Scatterv(CONST void *sendbuf,
		 CONST int *sendcnts,
		 CONST int *displs,
		 MPI_Datatype sendtype,
		 void *recvbuf,
		 int recvcnt,
		 MPI_Datatype recvtype,
		 int root,
		 MPI_Comm comm)
{
  FUNCTION_ENTRY;
  MPI_Scatterv_prolog(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm);
  int ret = MPI_Scatterv_core(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm);
  MPI_Scatterv_epilog(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt, recvtype, root, comm);
  return ret;
}


void mpif_scatterv_(void *sbuf, int *scount, int *displs, MPI_Fint *sd,
		    void *rbuf, int *rcount, MPI_Fint *rd,
		    int *root, MPI_Fint *c, int *error)
{
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Scatterv_prolog(sbuf, scount, displs, c_stype,
		      rbuf, *rcount, c_rtype,
		      *root, c_comm);

  *error = MPI_Scatterv_core(sbuf, scount, displs, c_stype,
			     rbuf, *rcount, c_rtype,
			     *root, c_comm);

  MPI_Scatterv_epilog(sbuf, scount, displs, c_stype,
		      rbuf, *rcount, c_rtype,
		      *root, c_comm);
}
