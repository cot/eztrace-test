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


static void MPI_Recv_prolog(void *buf __attribute__((unused)),
			    int count,
			    MPI_Datatype datatype,
			    int source,
			    int tag,
			    MPI_Comm comm,
			    MPI_Status *status __attribute__((unused)))
{
  int size;
  MPI_Type_size(datatype, &size);
  EZTRACE_EVENT4 (FUT_MPI_START_RECV, count*size, source, tag, comm);
}

static int MPI_Recv_core(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
  return libMPI_Recv(buf, count, datatype, source, tag, comm, status);
}

static void MPI_Recv_epilog(void *buf __attribute__((unused)),
			    int count,
			    MPI_Datatype datatype,
			    int source,
			    int tag,
			    MPI_Comm comm,
			    MPI_Status *status __attribute__((unused)))
{
  int size;
  MPI_Type_size(datatype, &size);
  EZTRACE_EVENT4 (FUT_MPI_STOP_RECV, count*size, source, tag, comm);
}

/* C function */
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
  FUNCTION_ENTRY;
  MPI_Recv_prolog(buf, count, datatype, source, tag, comm, status);
  int ret = MPI_Recv_core(buf, count, datatype, source, tag, comm, status);
  MPI_Recv_epilog(buf, count, datatype, source, tag, comm, status);
  return ret;
}

/* fortran function */
void mpif_recv_(void *buf, int *count, MPI_Fint *d,
		int *src, int *tag, MPI_Fint *c,
		MPI_Fint *s, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Status c_status;

  MPI_Recv_prolog(buf, *count, c_type, *src, *tag, c_comm, &c_status);
  *error = MPI_Recv_core(buf, *count, c_type, *src, *tag, c_comm, &c_status);
  MPI_Status_c2f(&c_status, s);
  MPI_Recv_epilog(buf, *count, c_type, *src, *tag, c_comm, &c_status);
}
