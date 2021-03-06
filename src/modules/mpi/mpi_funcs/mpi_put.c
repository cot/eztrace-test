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


static void MPI_Put_prolog(CONST void *origin_addr __attribute__((unused)),
			   int origin_count __attribute__((unused)),
			   MPI_Datatype origin_datatype,
			   int target_rank,
			   MPI_Aint target_disp __attribute__((unused)),
			   int target_count,
			   MPI_Datatype target_datatype,
			   MPI_Win win __attribute__((unused)))
{
  int osize;
  MPI_Type_size(origin_datatype, &osize);
  int tsize;
  MPI_Type_size(target_datatype, &tsize);

  /* FIXME: this is interpreted as a mpi_send, but it should not !
   */
  EZTRACE_EVENT3(FUT_MPI_START_PUT, target_count*tsize, target_rank, 0);
}

static int MPI_Put_core(CONST void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank,
			MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win)
{
  return libMPI_Put(origin_addr, origin_count, origin_datatype, target_rank,
		    target_disp, target_count, target_datatype, win);
}

static void MPI_Put_epilog(CONST void *origin_addr __attribute__((unused)),
			   int origin_count __attribute__((unused)),
			   MPI_Datatype origin_datatype __attribute__((unused)),
			   int target_rank __attribute__((unused)),
			   MPI_Aint target_disp __attribute__((unused)),
			   int target_count __attribute__((unused)),
			   MPI_Datatype target_datatype __attribute__((unused)),
			   MPI_Win win __attribute__((unused)))
{
  EZTRACE_EVENT0(FUT_MPI_STOP_PUT);
}


int MPI_Put(CONST void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank,
	    MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Win win)
{
  FUNCTION_ENTRY;

  MPI_Put_prolog(origin_addr, origin_count, origin_datatype, target_rank,
		 target_disp, target_count, target_datatype, win);
  int ret = MPI_Put_core(origin_addr, origin_count, origin_datatype, target_rank,
			 target_disp, target_count, target_datatype, win);
  MPI_Put_epilog(origin_addr, origin_count, origin_datatype, target_rank,
		 target_disp, target_count, target_datatype, win);
  return ret;
}


void mpif_put_(void *o_addr, int *o_count, MPI_Fint *o_d,
	       int *t_rank, MPI_Aint *t_disp, int *t_count,
	       MPI_Fint *t_d, MPI_Fint *win, int *error)
{
  MPI_Datatype c_otype = MPI_Type_f2c(*o_d);
  MPI_Datatype c_ttype = MPI_Type_f2c(*t_d);
  MPI_Win c_win = MPI_Win_f2c(*win);

  MPI_Put_prolog(o_addr, *o_count, c_otype, *t_rank, *t_disp, *t_count, c_ttype, c_win);
  *error = MPI_Put_core(o_addr, *o_count, c_otype, *t_rank, *t_disp, *t_count, c_ttype, c_win);
  MPI_Put_epilog(o_addr, *o_count, c_otype, *t_rank, *t_disp, *t_count, c_ttype, c_win);
}



