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


static void MPI_Sendrecv_replace_prolog (void* buf __attribute__((unused)),
					 int count,
					 MPI_Datatype type,
					 int dest,
					 int sendtag,
					 int src,
					 int recvtag,
					 MPI_Comm comm,
					 MPI_Status *status __attribute__((unused)))
{
  /* same as mpi_sendrecv but the same buffer is used for sending *and* receiving */
  int size;
  MPI_Type_size(type, &size);

  EZTRACE_EVENT4 (FUT_MPI_START_SENDRECV_REPLACE, count * size, src, recvtag, comm);
  EZTRACE_EVENT3 (FUT_MPI_Info, count*size, dest, sendtag);
}

static int MPI_Sendrecv_replace_core (void* buf, int count, MPI_Datatype type, int dest, int sendtag,
				      int src, int recvtag, MPI_Comm comm, MPI_Status *status)
{
  return libMPI_Sendrecv_replace (buf, count, type, dest, sendtag,
				  src, recvtag, comm, status);
}

static void MPI_Sendrecv_replace_epilog (void* buf __attribute__((unused)),
					 int count,
					 MPI_Datatype type,
					 int dest,
					 int sendtag,
					 int src,
					 int recvtag,
					 MPI_Comm comm,
					 MPI_Status *status __attribute__((unused)))
{
  int size;
  MPI_Type_size(type, &size);

  EZTRACE_EVENT4 (FUT_MPI_STOP_SENDRECV_REPLACE, count*size, dest, sendtag, comm);
  EZTRACE_EVENT3(FUT_MPI_Info, count * size, src, recvtag);
}




int MPI_Sendrecv_replace (void* buf, int count, MPI_Datatype type, int dest, int sendtag,
			  int src, int recvtag, MPI_Comm comm, MPI_Status *status)
{
  FUNCTION_ENTRY;
  MPI_Sendrecv_replace_prolog (buf, count, type, dest, sendtag,
			       src, recvtag, comm, status);
  int ret = MPI_Sendrecv_replace_core (buf, count, type, dest, sendtag,
				       src, recvtag, comm, status);
  MPI_Sendrecv_replace_epilog (buf, count, type, dest, sendtag,
			       src, recvtag, comm, status);
  return ret;
}


void mpif_sendrecv_replace_ (void* buf, int *count, MPI_Fint *type, int *dest, int *sendtag,
			     int *src, int *recvtag, MPI_Fint *comm, MPI_Status *status, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Datatype c_type = MPI_Type_f2c(*type);

  MPI_Sendrecv_replace_prolog (buf, *count, c_type, *dest, *sendtag,
			       *src, *recvtag, c_comm, status);
  *error = MPI_Sendrecv_replace_core (buf, *count, c_type, *dest, *sendtag,
				      *src, *recvtag, c_comm, status);
  MPI_Sendrecv_replace_epilog (buf, *count, c_type, *dest, *sendtag,
			       *src, *recvtag, c_comm, status);
}
