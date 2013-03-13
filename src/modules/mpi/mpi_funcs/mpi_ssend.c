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

static void MPI_Ssend_prolog (CONST void* buf __attribute__((unused)),
			      int count,
			      MPI_Datatype datatype,
			      int dest,
			      int tag,
			      MPI_Comm comm)
{
  int size;
  MPI_Type_size(datatype, &size);
  EZTRACE_EVENT4 (FUT_MPI_START_SSEND, count*size, dest, tag, comm);
}

static int MPI_Ssend_core (CONST void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
  return libMPI_Ssend(buf, count, datatype, dest, tag, comm);
}

static void MPI_Ssend_epilog (CONST void* buf  __attribute__((unused)),
			      int count  __attribute__((unused)),
			      MPI_Datatype datatype __attribute__((unused)),
			      int dest,
			      int tag,
			      MPI_Comm comm)
{
  EZTRACE_EVENT3 (FUT_MPI_STOP_SSEND, dest, tag, comm);
}


int MPI_Ssend (CONST void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
  FUNCTION_ENTRY;

  MPI_Ssend_prolog(buf, count, datatype, dest, tag, comm);
  int ret = MPI_Ssend_core(buf, count, datatype, dest, tag, comm);
  MPI_Ssend_epilog(buf, count, datatype, dest, tag, comm);
  return ret;
}

void mpif_ssend_(void *buf, int *count, MPI_Fint *d, int *dest,
		 int *tag, MPI_Fint *c, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Ssend_prolog(buf, *count, c_type, *dest, *tag, c_comm);
  *error = MPI_Ssend_core(buf, *count, c_type, *dest, *tag, c_comm);
  MPI_Ssend_epilog(buf, *count, c_type, *dest, *tag, c_comm);
}
