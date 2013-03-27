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


static void MPI_Send_prolog(CONST void *buf __attribute__((unused)),
			    int count,
			    MPI_Datatype datatype,
			    int dest,
			    int tag,
			    MPI_Comm comm)
{
  int size;
  /* retrieve the size of the datatype so that we can compute the message length */
  MPI_Type_size(datatype, &size);
  EZTRACE_EVENT4 (FUT_MPI_START_SEND, count*size, dest, tag, comm);
}

static int MPI_Send_core(CONST void *buf, int count, MPI_Datatype datatype,int dest, int tag, MPI_Comm comm)
{
  return libMPI_Send(buf, count, datatype, dest, tag, comm);
}

static void MPI_Send_epilog(CONST void *buf __attribute__((unused)),
			    int count __attribute__((unused)),
			    MPI_Datatype datatype __attribute__((unused)),
			    int dest,
			    int tag,
			    MPI_Comm comm)
{
  EZTRACE_EVENT3 (FUT_MPI_STOP_SEND, dest, tag, comm);
}

/* C function */
int MPI_Send(CONST void *buf, int count, MPI_Datatype datatype,int dest, int tag, MPI_Comm comm)
{
	int ret;
	FUNCTION_ENTRY;
	if(_UNUSED_MPI>1)
		ret = MPI_Send_core(buf, count, datatype, dest, tag, comm);
	else
	{
		MPI_Send_prolog(buf, count, datatype, dest, tag, comm);
		int ret = MPI_Send_core(buf, count, datatype, dest, tag, comm);
		MPI_Send_epilog(buf, count, datatype, dest, tag, comm);
	}
	return ret;
}

/* fortran function */
void mpif_send_(void* buf, int *count, MPI_Fint *d,
		int *dest, int *tag, MPI_Fint *c, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Send_prolog(buf, *count, c_type, *dest, *tag, c_comm);
  *error = MPI_Send_core(buf, *count, c_type, *dest, *tag, c_comm);
  MPI_Send_epilog(buf, *count, c_type, *dest, *tag, c_comm);
}
