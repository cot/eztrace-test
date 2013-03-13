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

static void MPI_Allgatherv_prolog (CONST void *sendbuf __attribute__((unused)),
				   int sendcount,
				   MPI_Datatype sendtype,
				   void *recvbuf __attribute__((unused)),
				   CONST int *recvcounts __attribute__((unused)),
				   CONST int *displs __attribute__((unused)),
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

  EZTRACE_EVENT4(FUT_MPI_START_Allgatherv, comm, size, rank, ssize*sendcount);

}

static int MPI_Allgatherv_core (CONST void *sendbuf __attribute__((unused)),
				int sendcount,
				MPI_Datatype sendtype,
				void *recvbuf __attribute__((unused)),
				CONST int *recvcounts __attribute__((unused)),
				CONST int *displs __attribute__((unused)),
				MPI_Datatype recvtype,
				MPI_Comm comm)
{
  return libMPI_Allgatherv (sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm);
}

static void MPI_Allgatherv_epilog (CONST void *sendbuf, int sendcount, MPI_Datatype sendtype,
				   void *recvbuf, CONST int *recvcounts, CONST int *displs,
				   MPI_Datatype recvtype, MPI_Comm comm)
{
 int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT4(FUT_MPI_STOP_Allgatherv, comm, size, rank, ssize*sendcount);
}


int MPI_Allgatherv (CONST void *sendbuf, int sendcount, MPI_Datatype sendtype,
		    void *recvbuf, CONST int *recvcounts, CONST int *displs,
                    MPI_Datatype recvtype, MPI_Comm comm)
{
  FUNCTION_ENTRY;
  MPI_Allgatherv_prolog (sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm);
  int ret = MPI_Allgatherv_core (sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm);
  MPI_Allgatherv_epilog (sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm);
  return ret;
}


void mpif_allgatherv_(void *sbuf, int *scount, MPI_Fint *sd,
		      void *rbuf, int *rcount, int *displs,
		      MPI_Fint *rd, MPI_Fint *c, int *error)
{
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Allgatherv_prolog(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, c_comm);
  *error = MPI_Allgatherv_core(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, c_comm);
  MPI_Allgatherv_epilog(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, c_comm);

}
