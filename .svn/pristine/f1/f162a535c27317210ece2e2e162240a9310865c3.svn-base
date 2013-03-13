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

static int MPI_Irecv_core (void* buf, int count, MPI_Datatype datatype, int src, int tag,
			   MPI_Comm comm, MPI_Request *req)
{
  return libMPI_Irecv(buf, count, datatype, src, tag, comm, req);
}

static void MPI_Irecv_epilog (void* buf __attribute__((unused)),
			      int count,
			      MPI_Datatype datatype,
			      int src,
			      int tag,
			      MPI_Comm comm,
			      MPI_Fint *req)
{
  int size;
  MPI_Type_size(datatype, &size);
  EZTRACE_EVENT5 (FUT_MPI_IRECV, count*size, src, tag, req, comm);
}

int MPI_Irecv (void* buf, int count, MPI_Datatype datatype, int src, int tag,
	       MPI_Comm comm, MPI_Request *req)
{
  FUNCTION_ENTRY;
  int ret = MPI_Irecv_core(buf, count, datatype, src, tag, comm, req);
  MPI_Irecv_epilog(buf, count, datatype, src, tag, comm, (MPI_Fint*)req);
  return ret;
}

void mpif_irecv_(void *buf, int *count, MPI_Fint *d, int *src,
		 int *tag, MPI_Fint *c, MPI_Fint *r, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Request c_req = MPI_Request_f2c(*r);

  int ret = MPI_Irecv_core(buf, *count, c_type, *src, *tag, c_comm, &c_req);
  *r= MPI_Request_c2f(c_req);
  MPI_Irecv_epilog(buf, *count, c_type, *src, *tag, c_comm, r);
}
