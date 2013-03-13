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


static int MPI_Recv_init_core(void* buffer, int count, MPI_Datatype type, int src, int tag,
			      MPI_Comm comm, MPI_Request *req)
{
  return libMPI_Recv_init(buffer, count, type, src, tag, comm, req);
}

static void MPI_Recv_init_epilog(void* buffer, int count, MPI_Datatype type, int src, int tag,
				 MPI_Comm comm, MPI_Fint *req)
{
  int size;
  MPI_Type_size(type, &size);
  EZTRACE_EVENT3(FUT_MPI_RECV_INIT, buffer, count*size, src);
  EZTRACE_EVENT3(FUT_MPI_Info, tag, comm, req);
}

int MPI_Recv_init(void* buffer, int count, MPI_Datatype type, int src, int tag,
		  MPI_Comm comm, MPI_Request *req)
{
  FUNCTION_ENTRY;
  int ret = MPI_Recv_init_core(buffer, count, type, src, tag, comm, req);
  MPI_Recv_init_epilog(buffer, count, type, src, tag, comm, (MPI_Fint*) req);
  return ret;
}


void mpif_recv_init_ (void*buffer, int*count, MPI_Fint*type, 
		      int*src, int*tag, MPI_Fint*comm, MPI_Fint *req, int*error)
{
  MPI_Datatype c_type = MPI_Type_f2c(*type);
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Request c_req = MPI_Request_f2c(*req);

  *error = MPI_Recv_init_core(buffer, *count, c_type, *src, *tag, c_comm, &c_req);
  *req = MPI_Request_c2f(c_req);
  MPI_Recv_init_epilog(buffer, *count, c_type, *src, *tag, c_comm, req);
}
