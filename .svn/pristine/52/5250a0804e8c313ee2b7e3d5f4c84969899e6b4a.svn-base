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


static void MPI_Waitall_prolog(int count,
			       void*req,
			       MPI_Status *s __attribute__((unused)),
			       size_t size)
{
  int i;
  /* we have to iterate over the array of requests so that eztrace_convert
   * know which requests the application is waiting for
   */
  EZTRACE_EVENT1(FUT_MPI_START_WAITALL, count);

  for(i=0; i< count; i++){
    /* we can't use &req[i] here since req may be an array of MPI_Request or MPI_Fint
     * (which may have different size)
     */
    EZTRACE_EVENT1(FUT_MPI_Info, (void*)req + (i*size));
  }
}

static int MPI_Waitall_core(int count, MPI_Request *req, MPI_Status *s)
{
  return libMPI_Waitall(count, req, s);
}

static void MPI_Waitall_epilog(int count,
			       void*req,
			       MPI_Status *s __attribute__((unused)),
			       size_t size)
{
  int i;
  EZTRACE_EVENT1(FUT_MPI_STOP_WAITALL, count);

  for(i=0;i<count; i++) {
    /* we can't use &req[i] here since req may be an array of MPI_Request or MPI_Fint
     * (which may have different size)
     */
    EZTRACE_EVENT1(FUT_MPI_Info, (void*)req + (i*size));
  }
}

int MPI_Waitall(int count, MPI_Request *req, MPI_Status *s)
{
  FUNCTION_ENTRY;
  MPI_Waitall_prolog(count, req, s, sizeof(MPI_Request));
  int ret = MPI_Waitall_core(count, req, s);
  MPI_Waitall_epilog(count, req, s, sizeof(MPI_Request));
  return ret;
}


void mpif_waitall_(int *c, MPI_Fint *r, MPI_Status *s, int *error)
{
  int i;
  MPI_Waitall_prolog(*c, r, s, sizeof(MPI_Fint));

  /* allocate a MPI_Request array and convert all the fortran requests
   * into C requests
   */
  ALLOCATE_ITEMS(MPI_Request, *c, c_req, p_req);
  for(i=0; i<*c;i++)
    p_req[i] = MPI_Request_f2c(r[i]);

  /* call the C version of MPI_Wait */
  *error = MPI_Waitall_core(*c, p_req, s);

  /* Since the requests may have been modified by MPI_Waitall,
   * we need to convert them back to Fortran
   */
  for(i=0; i<*c;i++)
    r[i]= MPI_Request_c2f(p_req[i]);


  MPI_Waitall_epilog(*c, r, s, sizeof(MPI_Fint));

  FREE_ITEMS(*c, p_req);
}
