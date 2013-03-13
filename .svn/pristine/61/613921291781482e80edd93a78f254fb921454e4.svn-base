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

static int MPI_Isend_core (CONST void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *req)
{
  return libMPI_Isend(buf, count, datatype, dest, tag, comm, req);
}

static void MPI_Isend_epilog (CONST void* buf __attribute__((unused)),
			      int count,
			      MPI_Datatype datatype,
			      int dest,
			      int tag,
			      MPI_Comm comm,
			      MPI_Fint *req)
{
  int size;
  MPI_Type_size(datatype, &size);
  EZTRACE_EVENT5 (FUT_MPI_ISEND, count * size, dest, tag, req, comm);
}

int MPI_Isend (CONST void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *req)
{
  FUNCTION_ENTRY;
  int ret = MPI_Isend_core(buf, count, datatype, dest, tag, comm, req);
  MPI_Isend_epilog(buf, count, datatype, dest, tag, comm, (MPI_Fint*)req);
  return ret;
}

void mpif_isend_(void *buf, int *count, MPI_Fint *d, int *dest,
		 int *tag, MPI_Fint *c, MPI_Fint *r, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Request c_req = MPI_Request_f2c(*r);

  *error = MPI_Isend_core(buf, *count, c_type, *dest, *tag, c_comm, &c_req);
  *r= MPI_Request_c2f(c_req);
  MPI_Isend_epilog(buf, *count, c_type, *dest, *tag, c_comm, r);
}
