/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include "mpi.h"
#include "mpi_eztrace.h"
#include "mpi_ev_codes.h"
#include "eztrace.h"
#include "mpi_eztrace.h"


/* For MPI_Init and MPI_Init_thread, we *have* to call the Fortran version
 * of the function. Once it is done, we can call the __mpi_init_generic function
 */
void __mpi_init_generic();

void mpif_init_(void *error)
{
  libmpi_init_((int*) error);
  __mpi_init_generic();
}

void mpif_init_thread_(int *r, int *p, int *error)
{
  libmpi_init_thread_(r, p, error);
  __mpi_init_generic();
}

/* For all the remaining Fortran functions, we can just call the C version */
void mpif_finalize_(int *error)
{
  *error = MPI_Finalize();
}

void mpif_comm_size_(MPI_Fint *c, int *s, int * error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  *error = MPI_Comm_size(c_comm, s);
}

void mpif_comm_rank_(MPI_Fint *c, int *r, int *error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  *error = MPI_Comm_rank(c_comm, r);
}



void mpif_comm_spawn_(char *command, char **argv, int *maxprocs,
		      MPI_Fint *info, int *root, MPI_Fint *comm,
		      MPI_Fint *intercomm, int *array_of_errcodes, int*error)
{
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Info c_info = MPI_Info_f2c(*info);
  ALLOCATE_ITEMS(MPI_Comm, *maxprocs, c_intercomm, p_intercomm);

  int i;
  for(i=0; i<*maxprocs;i++)
    p_intercomm[i] = MPI_Comm_f2c(intercomm[i]);

  *error = MPI_Comm_spawn(command, argv, *maxprocs,
			  c_info, *root, c_comm,
			  p_intercomm, array_of_errcodes);
  for(i=0; i<*maxprocs;i++)
    intercomm[i] = MPI_Comm_c2f(p_intercomm[i]);

  FREE_ITEMS(*maxprocs, p_intercomm);
}



void mpif_comm_create_(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *newcomm, int *error)
{
  MPI_Comm comm_c = MPI_Comm_f2c(*comm);
  MPI_Comm newcomm_c;

  MPI_Group group_c = MPI_Group_f2c(*group);

  *error = MPI_Comm_create(comm_c, group_c, &newcomm_c);
  *newcomm = MPI_Comm_c2f(newcomm_c);
}

void mpif_comm_split_(MPI_Fint *comm, int *color, int *key, MPI_Fint *newcomm, int *error)
{
  MPI_Comm comm_c = MPI_Comm_f2c(*comm);
  MPI_Comm newcomm_c;

  *error = MPI_Comm_split(comm_c, *color, *key, &newcomm_c);

  *newcomm = MPI_Comm_c2f(newcomm_c);
}

void mpif_comm_dup_(MPI_Fint *comm, MPI_Fint *newcomm, int *error)
{
  MPI_Comm comm_c = MPI_Comm_f2c(*comm);
  MPI_Comm newcomm_c;

  *error = MPI_Comm_dup(comm_c, &newcomm_c);

  *newcomm = MPI_Comm_c2f(newcomm_c);
}

void mpif_intercomm_create_(MPI_Fint *local_comm, int *local_leader, MPI_Fint *peer_comm, int *remote_leader, int *tag, MPI_Fint *newintercomm, int *error)
{
  MPI_Comm local_comm_c = MPI_Comm_f2c(*local_comm);
  MPI_Comm peer_comm_c = MPI_Comm_f2c(*peer_comm);
  MPI_Comm newintercomm_c;

  *error = MPI_Intercomm_create(local_comm_c, *local_leader, peer_comm_c, *remote_leader, *tag, &newintercomm_c);
  *newintercomm = MPI_Comm_c2f(newintercomm_c);
}

void mpif_intercomm_merge_(MPI_Fint *intercomm, int *high, MPI_Fint *newintracomm, int *error)
{
  MPI_Comm intercomm_c = MPI_Comm_f2c(*intercomm);
  MPI_Comm newintracomm_c;

  *error = MPI_Intercomm_merge(intercomm_c, *high, &newintracomm_c);
  *newintracomm = MPI_Comm_c2f(newintracomm_c);
}

void mpif_cart_sub_(MPI_Fint *old_comm, int *belongs, MPI_Fint *new_comm, int *error)
{
  MPI_Comm old_comm_c = MPI_Comm_f2c(*old_comm);
  MPI_Comm new_comm_c;

  *error = MPI_Cart_sub(old_comm_c, belongs, &new_comm_c);
  *new_comm = MPI_Comm_c2f(new_comm_c);
}

void mpif_cart_create_(MPI_Fint *comm_old, int *ndims, int *dims, int *periods, int *reorder, MPI_Fint *comm_cart, int *error)
{
  MPI_Comm comm_old_c = MPI_Comm_f2c(*comm_old);
  MPI_Comm comm_cart_c;

  *error = MPI_Cart_create(comm_old_c, *ndims, dims, periods, *reorder, &comm_cart_c);

  *comm_cart = MPI_Comm_c2f(comm_cart_c);
}

void mpif_graph_create_(MPI_Fint *comm_old, int *nnodes, int *index, int *edges, int *reorder, MPI_Fint *comm_graph, int *error)
{
  MPI_Comm comm_old_c = MPI_Comm_f2c(*comm_old);
  MPI_Comm comm_graph_c;

  *error = MPI_Graph_create(comm_old_c, *nnodes, index, edges, *reorder, &comm_graph_c);

  *comm_graph = MPI_Comm_c2f(comm_graph_c);
}
