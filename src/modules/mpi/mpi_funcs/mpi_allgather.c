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

static void MPI_Allgather_prolog(CONST void __attribute__((unused)) *sendbuf, int sendcount, MPI_Datatype sendtype,
				 void __attribute__((unused)) *recvbuf, int __attribute__((unused)) recvcount,
				 MPI_Datatype recvtype, MPI_Comm comm)
{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT4(FUT_MPI_START_Allgather, comm, size, rank, ssize*sendcount);
}

static int MPI_Allgather_core(CONST void *sendbuf, int sendcount, MPI_Datatype sendtype,
			      void *recvbuf, int recvcount, MPI_Datatype recvtype,
			      MPI_Comm comm)
{
  return libMPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}

static void MPI_Allgather_epilog(CONST void __attribute__((unused)) *sendbuf, int sendcount, MPI_Datatype sendtype,
				 void __attribute__((unused)) *recvbuf, int __attribute__((unused)) recvcount,
				 MPI_Datatype recvtype, MPI_Comm comm)

{
  int rank = -1;
  int size = -1;
  libMPI_Comm_size(comm, &size);
  libMPI_Comm_rank(comm, &rank);

  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);
  EZTRACE_EVENT4(FUT_MPI_STOP_Allgather, comm, size, rank, ssize*sendcount);

}

int MPI_Allgather(CONST void *sendbuf, int sendcount, MPI_Datatype sendtype,
		  void *recvbuf, int recvcount, MPI_Datatype recvtype,
		  MPI_Comm comm)
{
  FUNCTION_ENTRY;

  MPI_Allgather_prolog(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
  int ret = MPI_Allgather_core(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
  MPI_Allgather_epilog(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
  return ret;
}

void mpif_allgather_(void *sbuf, int *scount, MPI_Fint *sd,
		     void *rbuf, int *rcount, MPI_Fint *rd,
		     MPI_Fint *c, int *error)
{
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Allgather_prolog(sbuf, *scount, c_stype, rbuf, *rcount, c_rtype, c_comm);
  *error = MPI_Allgather_core(sbuf, *scount, c_stype, rbuf, *rcount, c_rtype, c_comm);
  MPI_Allgather_epilog(sbuf, *scount, c_stype, rbuf, *rcount, c_rtype, c_comm);
}
