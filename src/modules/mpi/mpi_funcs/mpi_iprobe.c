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



static int MPI_Iprobe_core( int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status )
{
  return libMPI_Iprobe(source, tag, comm, flag, status);
}


static void MPI_Iprobe_epilog( int source __attribute__((unused)),
			       int tag __attribute__((unused)),
			       MPI_Comm comm __attribute__((unused)),
			       int *flag,
			       MPI_Status *status )
{
  if(*flag) {
    int length = -1;
    MPI_Get_count(status, MPI_BYTE, &length);
    EZTRACE_EVENT3(FUT_MPI_IPROBE_SUCCESS, status->MPI_SOURCE, status->MPI_TAG, length);
  }
#if 0
  /* comment out in order to avoid spamming the output trace
   * when the application perform a busy waiting like this:
   * do { iprobe(); } while(!success);
   */
  else {
    EZTRACE_EVENT2(FUT_MPI_IPROBE_FAILED, source, tag);
  }
#endif

}

int MPI_Iprobe( int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status )
{
  FUNCTION_ENTRY;
  int ret = MPI_Iprobe_core(source, tag, comm, flag, status);
  MPI_Iprobe_epilog(source, tag, comm, flag, status);

  return ret;
}


void mpif_iprobe_( int* source, int* tag, MPI_Fint* comm, int *flag, MPI_Status *status, int* err )
{
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  *err = MPI_Iprobe_core(*source, *tag, c_comm, flag, status);
  MPI_Iprobe_epilog(*source, *tag, c_comm, flag, status);
}
