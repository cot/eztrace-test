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


static void MPI_Sendrecv_prolog (CONST void *sendbuf __attribute__((unused)),
				 int sendcount,
				 MPI_Datatype sendtype,
				 int dest,
				 int sendtag,
				 void *recvbuf __attribute__((unused)),
				 int recvcount,
				 MPI_Datatype recvtype,
				 int src,
				 int recvtag,
				 MPI_Comm comm,
				 MPI_Status *status __attribute__((unused)))
{
  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT4 (FUT_MPI_START_SENDRECV, recvcount * rsize, src, recvtag, comm);
  EZTRACE_EVENT3(FUT_MPI_Info, sendcount*ssize, dest, sendtag);
}

static int MPI_Sendrecv_core (CONST void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag,
			      void *recvbuf, int recvcount, MPI_Datatype recvtype, int src, int recvtag,
			      MPI_Comm comm, MPI_Status *status)
{
  return libMPI_Sendrecv (sendbuf, sendcount, sendtype, dest, sendtag,
			  recvbuf, recvcount, recvtype, src, recvtag,
			  comm, status);
}

static void MPI_Sendrecv_epilog (CONST void *sendbuf __attribute__((unused)),
				 int sendcount __attribute__((unused)),
				 MPI_Datatype sendtype __attribute__((unused)),
				 int dest __attribute__((unused)),
				 int sendtag __attribute__((unused)),
				 void *recvbuf __attribute__((unused)),
				 int recvcount,
				 MPI_Datatype recvtype,
				 int src,
				 int recvtag,
				 MPI_Comm comm,
				 MPI_Status *status __attribute__((unused)))
{
  int ssize, rsize;
  MPI_Type_size(sendtype, &ssize);
  MPI_Type_size(recvtype, &rsize);

  EZTRACE_EVENT4 (FUT_MPI_STOP_SENDRECV, sendcount*ssize, dest, sendtag, comm);
  EZTRACE_EVENT3(FUT_MPI_Info, recvcount * rsize, src, recvtag);
}

int MPI_Sendrecv (CONST void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag,
		  void *recvbuf, int recvcount, MPI_Datatype recvtype, int src, int recvtag,
		  MPI_Comm comm, MPI_Status *status)
{
  FUNCTION_ENTRY;

  MPI_Sendrecv_prolog (sendbuf, sendcount, sendtype, dest, sendtag,
		       recvbuf, recvcount, recvtype, src, recvtag,
		       comm, status);
  int ret = MPI_Sendrecv_core (sendbuf, sendcount, sendtype, dest, sendtag,
			       recvbuf, recvcount, recvtype, src, recvtag,
			       comm, status);

  MPI_Sendrecv_epilog (sendbuf, sendcount, sendtype, dest, sendtag,
		       recvbuf, recvcount, recvtype, src, recvtag,
		       comm, status);

  return ret;
}


void mpif_sendrecv_ (void *sendbuf, int *sendcount, MPI_Fint *sendtype, int *dest, int *sendtag,
		     void *recvbuf, int *recvcount, MPI_Fint *recvtype, int *src, int *recvtag,
		     MPI_Fint *comm, MPI_Status *status, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Datatype c_stype = MPI_Type_f2c(*sendtype);
  MPI_Datatype c_rtype = MPI_Type_f2c(*recvtype);

  MPI_Sendrecv_prolog (sendbuf, *sendcount, c_stype, *dest, *sendtag,
		       recvbuf, *recvcount, c_rtype, *src, *recvtag,
		       c_comm, status);
  *error = MPI_Sendrecv_core (sendbuf, *sendcount, c_stype, *dest, *sendtag,
			      recvbuf, *recvcount, c_rtype, *src, *recvtag,
			      c_comm, status);
  MPI_Sendrecv_epilog (sendbuf, *sendcount, c_stype, *dest, *sendtag,
		       recvbuf, *recvcount, c_rtype, *src, *recvtag,
		       c_comm, status);
}
