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

#include "pptrace.h"
#include "mpi.h"
#include "mpi_eztrace.h"
#include "mpi_ev_codes.h"
#include "eztrace.h"


/* Process identifier.
 * It corresponds to the global MPI rank unless the process was spawned.
 * In that case, the identifier is the concatenation of the parent process id
 * and the global rank.
 * For example process id 0_1_3 has a global rank of 3 and is has been spawned by process 0_1
 * Process 0_1 has a global rank of 1 and was spawned by process 0
 */
char *proc_id;


/* pointers to actual MPI functions (C version)  */
int ( *libMPI_Init) (int *, char ***);
int ( *libMPI_Init_thread) (int *, char ***, int, int*);
int ( *libMPI_Comm_size) (MPI_Comm, int *);
int ( *libMPI_Comm_rank) (MPI_Comm, int *);
int ( *libMPI_Comm_get_parent) (MPI_Comm *parent) = NULL;
int ( *libMPI_Finalize) (void);
int ( *libMPI_Initialized) (int *);
int ( *libMPI_Abort) (MPI_Comm, int);
int ( *libMPI_Type_size)(MPI_Datatype datatype, int *size);

int ( *libMPI_Comm_create)(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
int ( *libMPI_Comm_split)(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
int ( *libMPI_Comm_dup)(MPI_Comm comm, MPI_Comm *newcomm);
int ( *libMPI_Intercomm_create)(MPI_Comm local_comm, int local_leader, MPI_Comm peer_comm, int remote_leader, int tag, MPI_Comm *newintercomm);
int ( *libMPI_Intercomm_merge)(MPI_Comm intercomm, int high, MPI_Comm *newintracomm);
int ( *libMPI_Cart_sub)(MPI_Comm old_comm, CONST int *belongs, MPI_Comm *new_comm);
int ( *libMPI_Cart_create)(MPI_Comm comm_old, int ndims, CONST int *dims, CONST int *periods, int reorder, MPI_Comm *comm_cart);
int ( *libMPI_Graph_create)(MPI_Comm comm_old, int nnodes, CONST int *index, CONST int *edges, int reorder, MPI_Comm *comm_graph);

int ( *libMPI_Send) (CONST void *buf, int count, MPI_Datatype datatype,int dest, int tag,MPI_Comm comm);
int ( *libMPI_Recv) (void *buf, int count, MPI_Datatype datatype,int source, int tag, MPI_Comm comm, MPI_Status *status);

int ( *libMPI_Bsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
int ( *libMPI_Ssend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
int ( *libMPI_Rsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
int ( *libMPI_Isend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int ( *libMPI_Ibsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int ( *libMPI_Issend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int ( *libMPI_Irsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int ( *libMPI_Irecv) (void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);

int ( *libMPI_Sendrecv) (CONST void *, int, MPI_Datatype,int, int, void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
int ( *libMPI_Sendrecv_replace) (void*, int, MPI_Datatype, int, int, int, int, MPI_Comm, MPI_Status *);

int ( *libMPI_Send_init) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
int ( *libMPI_Bsend_init) (CONST void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int ( *libMPI_Ssend_init) (CONST void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int ( *libMPI_Rsend_init) (CONST void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int ( *libMPI_Recv_init) (void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
int ( *libMPI_Start) (MPI_Request *);
int ( *libMPI_Startall) (int, MPI_Request *);

int ( *libMPI_Wait) (MPI_Request *, MPI_Status *);
int ( *libMPI_Test) (MPI_Request *, int *, MPI_Status *);
int ( *libMPI_Waitany) (int, MPI_Request *, int *, MPI_Status *);
int ( *libMPI_Testany) (int, MPI_Request *, int *, int *, MPI_Status *);
int ( *libMPI_Waitall) (int, MPI_Request *, MPI_Status *);
int ( *libMPI_Testall) (int, MPI_Request *, int *, MPI_Status *);
int ( *libMPI_Waitsome) (int, MPI_Request *, int *, int *, MPI_Status *);
int ( *libMPI_Testsome) (int, MPI_Request *, int *, int *, MPI_Status *);

int ( *libMPI_Probe)( int source, int tag, MPI_Comm comm, MPI_Status *status );
int ( *libMPI_Iprobe)( int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status );

int ( *libMPI_Barrier) (MPI_Comm );
int ( *libMPI_Bcast) (void*, int, MPI_Datatype, int, MPI_Comm );
int ( *libMPI_Gather) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm);
int ( *libMPI_Gatherv) (CONST void* , int, MPI_Datatype, void*, CONST int *, CONST int *, MPI_Datatype, int, MPI_Comm);
int ( *libMPI_Scatter) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm);
int ( *libMPI_Scatterv) (CONST void* , CONST int *, CONST int *,  MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm);
int ( *libMPI_Allgather) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, MPI_Comm);
int ( *libMPI_Allgatherv) (CONST void* , int, MPI_Datatype, void*, CONST int *, CONST int *, MPI_Datatype, MPI_Comm);
int ( *libMPI_Alltoall) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, MPI_Comm);
int ( *libMPI_Alltoallv) (CONST void* , CONST int *, CONST int *, MPI_Datatype, void*, CONST int *, CONST int *, MPI_Datatype, MPI_Comm);
int ( *libMPI_Reduce) (CONST void* , void*, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
int ( *libMPI_Allreduce) (CONST void* , void*, int, MPI_Datatype, MPI_Op, MPI_Comm);
int ( *libMPI_Reduce_scatter) (CONST void* , void*, CONST int *, MPI_Datatype, MPI_Op, MPI_Comm);
int ( *libMPI_Scan) (CONST void* , void*, int, MPI_Datatype, MPI_Op, MPI_Comm );


int ( *libMPI_Get) (void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
			   MPI_Win);
int ( *libMPI_Put) (CONST void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
			   MPI_Win);

int ( *libMPI_Comm_spawn)(CONST char *command,
			  char *argv[],
			  int maxprocs,
			  MPI_Info info,
			  int root,
			  MPI_Comm comm,
			  MPI_Comm *intercomm,
			  int array_of_errcodes[]);

/* fortran bindings */
void (*libmpi_init_)(int*e);
void (*libmpi_init_thread_)(int*, int*, int*);
void (*libmpi_finalize_)(int*);
void (*libmpi_barrier_)(MPI_Comm*, int*);
void (*libmpi_comm_size_)(MPI_Comm*, int*, int*);
void (*libmpi_comm_rank_)(MPI_Comm*, int*, int*);
void (*libmpi_comm_get_parent_) (MPI_Comm *, int*);
void (*libmpi_type_size_)(MPI_Datatype *, int *, int*);

int ( *libmpi_comm_create_)(int*, int*, int*, int*);
int ( *libmpi_comm_split_)(int*, int*, int*, int*, int*);
int ( *libmpi_comm_dup_)(int*, int*, int*);
int ( *libmpi_intercomm_create_)(int*, int*, int*, int*, int*, int*, int*);
int ( *libmpi_intercomm_merge_)(int*, int*, int*, int*);
int ( *libmpi_cart_sub_)(int*, int*, int*, int*);
int ( *libmpi_cart_create_)(int*, int*, int*, int*, int*, int*, int*);
int ( *libmpi_graph_create_)(int*, int*, int*, int*, int*, int*, int*);


void (*libmpi_send_)(void*, int*, MPI_Datatype*, int*, int*, int*);
void (*libmpi_recv_)(void*, int*, MPI_Datatype*, int*, int *, MPI_Status *, int*);

void ( *libmpi_sendrecv_) (void *, int, MPI_Datatype,int, int, void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *, int*);
void ( *libmpi_sendrecv_replace_) (void*, int, MPI_Datatype, int, int, int, int, MPI_Comm, MPI_Status *, int*);

void (*libmpi_bsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
void (*libmpi_ssend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
void (*libmpi_rsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
void (*libmpi_isend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request*, int*);
void (*libmpi_ibsend_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request*, int*);
void (*libmpi_issend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
void (*libmpi_irsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
void (*libmpi_irecv_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *,  int*);

void (*libmpi_wait_)(MPI_Request*, MPI_Status*, int*);
void (*libmpi_test_)(MPI_Request*, int*, MPI_Status*, int*);
void (*libmpi_waitany_) (int*, MPI_Request *, int *, MPI_Status *, int*);
void (*libmpi_testany_) (int*, MPI_Request *, int *, int *, MPI_Status *, int*);
void (*libmpi_waitall_) (int*, MPI_Request *, MPI_Status *, int*);
void (*libmpi_testall_) (int*, MPI_Request *, int *, MPI_Status *, int*);
void (*libmpi_waitsome_) (int*, MPI_Request *, int *, int *, MPI_Status *, int*);
void (*libmpi_testsome_) (int*, MPI_Request *, int *, int *, MPI_Status *, int*);

void ( *libmpi_probe_)( int* source, int* tag, MPI_Comm* comm, MPI_Status *status, int* err );
void ( *libmpi_iprobe_)( int* source, int* tag, MPI_Comm* comm, int *flag, MPI_Status *status, int* err );

void (*libmpi_get_)(void *, int*, MPI_Datatype*, int*, MPI_Aint*, int*, MPI_Datatype*, MPI_Win*, int*);
void (*libmpi_put_)(void *, int*, MPI_Datatype*, int*, MPI_Aint*, int*, MPI_Datatype*, MPI_Win*, int*);

void (*libmpi_bcast_)(void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
void (*libmpi_gather_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
void (*libmpi_gatherv_)(void*, int*, MPI_Datatype*, void*, int*, int*, MPI_Datatype*, int*, MPI_Comm*);
void (*libmpi_scatter_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
void (*libmpi_scatterv_)(void*, int*, int*,  MPI_Datatype*, void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
void (*libmpi_allgather_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, MPI_Comm*, int*);
void (*libmpi_allgatherv_)(void*, int*, MPI_Datatype*, void*, int*, int*, MPI_Datatype*, MPI_Comm*);
void (*libmpi_alltoall_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, MPI_Comm*, int*);
void (*libmpi_alltoallv_)(void*, int*, int*, MPI_Datatype*, void*, int*, int*, MPI_Datatype*, MPI_Comm*, int*);
void (*libmpi_reduce_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, int*, MPI_Comm*, int*);
void (*libmpi_allreduce_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*, int*);
void (*libmpi_reduce_scatter_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*, int*);
void (*libmpi_scan_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*, int*);



void (*libmpi_comm_spawn_)(char *command, char **argv, int *maxprocs,
			   MPI_Info *info, int *root, MPI_Comm *comm,
			   MPI_Comm *intercomm, int *array_of_errcodes, int*error);

void ( *libmpi_send_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
void ( *libmpi_bsend_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
void ( *libmpi_ssend_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
void ( *libmpi_rsend_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
void ( *libmpi_recv_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
void ( *libmpi_start_) (MPI_Request *, int*);
void ( *libmpi_startall_) (int*, MPI_Request *, int*);



/* Functions that intercept MPI calls
 * Basically each function create an event this the arguments
 * passed to the function.
 * It then call the actual MPI function (using the appropriate
 * callback) with the same args
 */
int MPI_Comm_spawn(CONST char *command,
		   char *argv[],
		   int maxprocs,
		   MPI_Info info,
		   int root,
		   MPI_Comm comm,
		   MPI_Comm *intercomm,
		   int array_of_errcodes[])
{
  /* Instead of running command argv, we have to 
     run 'env LD_PRELOAD=xxx command argv'
     Thus, we have to provide a new argv array
  */

  /* retrieve LD_PRELOAD command set by EZTrace */
  char* ld_preload = getenv(LD_PRELOAD_NAME);
  char* ld_preload_str = NULL;
  int ret = asprintf(&ld_preload_str, "%s=%s", LD_PRELOAD_NAME, ld_preload);

  /* count the number of args */
  int argc = 0;
  if(argv != MPI_ARGV_NULL)
    for(argc=0; argv[argc] != NULL; argc++) { }

  /* create a new argv array */
  int new_argc = argc+3;
  char **new_argv = (char**) malloc(sizeof(char*) * new_argc );

  new_argv[0] = ld_preload_str;
  asprintf(&new_argv[1], "%s", command);
  int i;
  for(i=0; i<argc; i++)
    new_argv[i+2] = argv[i];

  new_argv[i+2]=NULL;

  ret = libMPI_Comm_spawn("env", new_argv, maxprocs, info, root, comm, intercomm, array_of_errcodes);

  /* Now that the processes are launched, tell them our proc_id so that the filenames are not messed up */
  int f_size; 			/* number of children actually created */
  int proc_id_len = strlen(proc_id) + 1;
  int my_pid = getpid();
  MPI_Comm_remote_size(*intercomm, &f_size);

  EZTRACE_EVENT2 (FUT_MPI_SPAWN, my_pid, f_size);

  for(i=0; i<f_size; i++) {
    MPI_Send(&proc_id_len, 1 , MPI_INTEGER, i, 0, *intercomm);
    MPI_Send(proc_id, proc_id_len , MPI_CHAR, i, 0, *intercomm);
    MPI_Send(&my_pid, 1 , MPI_INTEGER, i, 0, *intercomm);
  }

  /* Here, we shall not free ld_preload, since it may modify the environment of the process ! (man getenv) */
  free(new_argv);
  free(ld_preload_str);

  FUNCTION_ENTRY;
  return ret;
}

int MPI_Comm_get_parent(MPI_Comm *parent)
{
  if(!libMPI_Comm_get_parent) {
    /* MPI_Comm_get_parent was not found. Let's assume the application doesn't use it. */
    *parent = MPI_COMM_NULL;
    return MPI_SUCCESS;
  }
  return libMPI_Comm_get_parent(parent);
}

int MPI_Comm_size(MPI_Comm c, int *s)
{
  return libMPI_Comm_size(c, s);
}

int MPI_Comm_rank(MPI_Comm c, int *r)
{
  return libMPI_Comm_rank(c, r);
}

int MPI_Type_size(MPI_Datatype datatype, int *size)
{
  return libMPI_Type_size(datatype, size);
}

int MPI_Finalize()
{
  FUNCTION_ENTRY;
  return libMPI_Finalize();
}

/* internal function
 * This function is used by the various MPI_Init* functions (C 
 * and Fortran versions)
 * This function add informations to the trace (rank, etc.)
 * and set the trace filename.
 */
void __mpi_init_generic()
{
  int rank = -1;
  int size = -1;
  int ret;
  static int __mpi_initialized = 0;

  MPI_Comm parentcomm;
  MPI_Comm_get_parent( &parentcomm );

  libMPI_Comm_size(MPI_COMM_WORLD, &size);
  libMPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char* filename=NULL;
  if(parentcomm == MPI_COMM_NULL) {
    /* This process is a 'normal' process (ie. it wasn't spawned) */
    ret = asprintf(&proc_id, "%d", rank);
  } else {
    /* This process was spawned.
     * We have to get the parent process information
     */
    char *father_proc_id;
    int father_proc_id_len = -1;
    int ppid = -1;

    /* Get the parent process id */
    libMPI_Recv(&father_proc_id_len, 1, MPI_INTEGER, 0, 0, parentcomm, MPI_STATUS_IGNORE);
    father_proc_id = (char*) malloc(sizeof(char) * father_proc_id_len);
    libMPI_Recv(father_proc_id, father_proc_id_len, MPI_CHAR, 0, 0, parentcomm, MPI_STATUS_IGNORE);

    libMPI_Recv(&ppid, 1, MPI_INTEGER, 0, 0, parentcomm, MPI_STATUS_IGNORE);

    if(!__mpi_initialized)
      EZTRACE_EVENT2 (FUT_MPI_SPAWNED, ppid, rank);

    ret = asprintf(&proc_id, "%s_%d", father_proc_id, rank);
    free(father_proc_id);
  }

  ret = asprintf(&filename, "eztrace_log_rank_%s", proc_id);
  eztrace_set_filename(filename);

  libMPI_Barrier(MPI_COMM_WORLD);
  if(!__mpi_initialized) {
    EZTRACE_EVENT5 (FUT_MPI_INIT, rank, size, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_REQUEST_NULL);
    EZTRACE_EVENT2 (FUT_MPI_INIT_Info, MPI_COMM_WORLD, MPI_COMM_SELF);
  }

  __mpi_initialized = 1;
}

int MPI_Init_thread (int *argc, char ***argv, int required, int *provided)
{
  int ret = libMPI_Init_thread(argc, argv, required, provided);
  __mpi_init_generic();
  FUNCTION_ENTRY;
  return ret;
}


int MPI_Init(int * argc, char***argv)
{
  int ret = libMPI_Init(argc, argv);
  __mpi_init_generic();
  FUNCTION_ENTRY;
  return ret;
}

static void __ezt_new_mpi_comm(MPI_Comm comm)
{
  MPI_Group world_group, group;
  int gsize; 			/* size of the group */

  MPI_Comm_group(MPI_COMM_WORLD, &world_group);
  MPI_Comm_group(comm, &group);

  MPI_Group_size(group, &gsize);

  int *local_ranks = malloc(sizeof(int)*gsize);
  int *global_ranks = malloc(sizeof(int)*gsize);
  int i;
  for(i=0;i<gsize; i++)
    local_ranks[i]=i;

  /* translate the ranks of the local group into rank of the world group */
  MPI_Group_translate_ranks(group, gsize, local_ranks, world_group, global_ranks);

  /* record the information we need to use the communicator */
  EZTRACE_EVENT2 (FUT_MPI_NEW_COMM, comm, gsize);
  for(i=0; i<gsize; i++) {
    EZTRACE_EVENT1 (FUT_MPI_NEW_COMM_Info, global_ranks[i]);
  }
}


#ifdef DEBUG
#define CHECK_COMMUNICATOR(newcomm, comm)				\
  do {									\
    if(newcomm == MPI_COMM_NULL) {					\
      int rank, size;							\
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);				\
      MPI_Comm_size(MPI_COMM_WORLD, &size);				\
      fprintf(stderr, "Warning: new communicator (%x) is NULL ! This communicator was created from communicator %x using %s\n", \
	      , newcomm, comm, __FUNCTION__);				\
    }									\
  } while(0)
#else
#define CHECK_COMMUNICATOR(newcomm, comm) (void)(0)
#endif


int MPI_Comm_create(MPI_Comm comm,
		    MPI_Group group,
		    MPI_Comm *newcomm)
{
  int ret = libMPI_Comm_create(comm, group, newcomm);
  if(ret == MPI_SUCCESS) {
    CHECK_COMMUNICATOR(*newcomm, comm);
    __ezt_new_mpi_comm(*newcomm);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}

int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
  int ret = libMPI_Comm_split(comm, color, key, newcomm);
  if(ret == MPI_SUCCESS) {
    if(*newcomm != MPI_COMM_NULL)
      __ezt_new_mpi_comm(*newcomm);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
  int ret = libMPI_Comm_dup(comm, newcomm);
  if(ret == MPI_SUCCESS) {
    CHECK_COMMUNICATOR(*newcomm, comm);
    __ezt_new_mpi_comm(*newcomm);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}

int MPI_Intercomm_create(MPI_Comm local_comm, int local_leader, MPI_Comm peer_comm,
			 int remote_leader, int tag, MPI_Comm *newintercomm)
{
  int ret = libMPI_Intercomm_create(local_comm, local_leader, peer_comm, remote_leader, tag, newintercomm);
  if(ret == MPI_SUCCESS) {
    CHECK_COMMUNICATOR(*newintercomm, local_comm);
    __ezt_new_mpi_comm(*newintercomm);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}

int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm *newintracomm)
{
  int ret = libMPI_Intercomm_merge(intercomm, high, newintracomm);
  if(ret == MPI_SUCCESS) {
    CHECK_COMMUNICATOR(*newintracomm, intercomm);
    __ezt_new_mpi_comm(*newintracomm);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}

int MPI_Cart_sub(MPI_Comm old_comm, CONST int *belongs, MPI_Comm *new_comm)
{
  int ret = libMPI_Cart_sub(old_comm, belongs, new_comm);
  if(ret == MPI_SUCCESS) {
    CHECK_COMMUNICATOR(*new_comm, old_comm);
    __ezt_new_mpi_comm(*new_comm);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}

int MPI_Cart_create(MPI_Comm comm_old, int ndims, CONST int *dims, CONST int *periods, int reorder, MPI_Comm *comm_cart)
{
  int ret =  libMPI_Cart_create(comm_old, ndims, dims, periods, reorder, comm_cart);
  if(ret == MPI_SUCCESS) {
    CHECK_COMMUNICATOR(*comm_cart, comm_old);
    __ezt_new_mpi_comm(*comm_cart);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}

int MPI_Graph_create(MPI_Comm comm_old, int nnodes, CONST int *index, CONST int *edges, int reorder, MPI_Comm *comm_graph)
{
  int ret = libMPI_Graph_create(comm_old, nnodes, index, edges, reorder, comm_graph);
  if(ret == MPI_SUCCESS) {
    CHECK_COMMUNICATOR(*comm_graph, comm_old);
    __ezt_new_mpi_comm(*comm_graph);
  } else {
    fprintf(stderr, "Warning: %s returned %d\n", __FUNCTION__, ret);
  }
  return ret;
}


START_INTERCEPT
  INTERCEPT2("MPI_Init_thread", libMPI_Init_thread)
  INTERCEPT2("MPI_Init", libMPI_Init)
  INTERCEPT2("MPI_Finalize", libMPI_Finalize)
  INTERCEPT2("MPI_Barrier", libMPI_Barrier)
  INTERCEPT2("MPI_Comm_size", libMPI_Comm_size)
  INTERCEPT2("MPI_Comm_rank", libMPI_Comm_rank)
  INTERCEPT2("MPI_Comm_get_parent", libMPI_Comm_get_parent)
  INTERCEPT2("MPI_Type_size", libMPI_Type_size)

  INTERCEPT2("MPI_Comm_create", libMPI_Comm_create)
  INTERCEPT2("MPI_Comm_split", libMPI_Comm_split)
  INTERCEPT2("MPI_Comm_dup", libMPI_Comm_dup)
  INTERCEPT2("MPI_Intercomm_create", libMPI_Intercomm_create)
  INTERCEPT2("MPI_Intercomm_merge", libMPI_Intercomm_merge)
  INTERCEPT2("MPI_Cart_sub", libMPI_Cart_sub)
  INTERCEPT2("MPI_Cart_create", libMPI_Cart_create)
  INTERCEPT2("MPI_Graph_create", libMPI_Graph_create)

  INTERCEPT2("MPI_Send", libMPI_Send)
  INTERCEPT2("MPI_Recv", libMPI_Recv)

  INTERCEPT2("MPI_Sendrecv", libMPI_Sendrecv)
  INTERCEPT2("MPI_Sendrecv_replace", libMPI_Sendrecv_replace)

  INTERCEPT2("MPI_Bsend", libMPI_Bsend)
  INTERCEPT2("MPI_Ssend", libMPI_Ssend)
  INTERCEPT2("MPI_Rsend", libMPI_Rsend)
  INTERCEPT2("MPI_Isend", libMPI_Isend)
  INTERCEPT2("MPI_Ibsend", libMPI_Ibsend)
  INTERCEPT2("MPI_Issend", libMPI_Issend)
  INTERCEPT2("MPI_Irsend", libMPI_Irsend)
  INTERCEPT2("MPI_Irecv", libMPI_Irecv)

  INTERCEPT2("MPI_Wait", libMPI_Wait)
  INTERCEPT2("MPI_Waitall", libMPI_Waitall)
  INTERCEPT2("MPI_Waitany", libMPI_Waitany)
  INTERCEPT2("MPI_Waitsome", libMPI_Waitsome)
  INTERCEPT2("MPI_Test", libMPI_Test)
  INTERCEPT2("MPI_Testall", libMPI_Testall)
  INTERCEPT2("MPI_Testany", libMPI_Testany)
  INTERCEPT2("MPI_Testsome", libMPI_Testsome)

  INTERCEPT2("MPI_Iprobe", libMPI_Iprobe)
  INTERCEPT2("MPI_Probe", libMPI_Probe)

  INTERCEPT2("MPI_Get", libMPI_Get)
  INTERCEPT2("MPI_Put", libMPI_Put)

  INTERCEPT2("MPI_Bcast", libMPI_Bcast)
  INTERCEPT2("MPI_Gather", libMPI_Gather)
  INTERCEPT2("MPI_Gatherv", libMPI_Gatherv)
  INTERCEPT2("MPI_Scatter", libMPI_Scatter)
  INTERCEPT2("MPI_Scatterv", libMPI_Scatterv)
  INTERCEPT2("MPI_Allgather", libMPI_Allgather)
  INTERCEPT2("MPI_Allgatherv", libMPI_Allgatherv)
  INTERCEPT2("MPI_Alltoall", libMPI_Alltoall)
  INTERCEPT2("MPI_Alltoallv", libMPI_Alltoallv)
  INTERCEPT2("MPI_Reduce", libMPI_Reduce)
  INTERCEPT2("MPI_Allreduce", libMPI_Allreduce)
  INTERCEPT2("MPI_Reduce_scatter", libMPI_Reduce_scatter)
  INTERCEPT2("MPI_Scan", libMPI_Scan)

  INTERCEPT2("MPI_Comm_spawn", libMPI_Comm_spawn)

  INTERCEPT2("MPI_Send_init", libMPI_Send_init)
  INTERCEPT2("MPI_Bsend_init", libMPI_Bsend_init)
  INTERCEPT2("MPI_Ssend_init", libMPI_Ssend_init)
  INTERCEPT2("MPI_Rsend_init", libMPI_Rsend_init)
  INTERCEPT2("MPI_Recv_init", libMPI_Recv_init)
  INTERCEPT2("MPI_Start", libMPI_Start)
  INTERCEPT2("MPI_Startall", libMPI_Startall)

  /* fortran binding */
  INTERCEPT2("mpi_init_", libmpi_init_)
  INTERCEPT2("mpi_init_thread_", libmpi_init_thread_)
  INTERCEPT2("mpi_init_", libmpi_init_)
  INTERCEPT2("mpi_finalize_", libmpi_finalize_)
  INTERCEPT2("mpi_barrier_", libmpi_barrier_)
  INTERCEPT2("mpi_comm_size_", libmpi_comm_size_)
  INTERCEPT2("mpi_comm_rank_", libmpi_comm_rank_)
  INTERCEPT2("mpi_comm_get_parent_", libmpi_comm_get_parent_)
  INTERCEPT2("mpi_type_size_", libmpi_type_size_)

  INTERCEPT2("mpi_comm_create", libmpi_comm_create_)
  INTERCEPT2("mpi_comm_split", libmpi_comm_split_)
  INTERCEPT2("mpi_comm_dup", libmpi_comm_dup_)
  INTERCEPT2("mpi_intercomm_create", libmpi_intercomm_create_)
  INTERCEPT2("mpi_intercomm_merge", libmpi_intercomm_merge_)
  INTERCEPT2("mpi_cart_sub", libmpi_cart_sub_)
  INTERCEPT2("mpi_cart_create", libmpi_cart_create_)
  INTERCEPT2("mpi_graph_create", libmpi_graph_create_)


  INTERCEPT2("mpi_send_", libmpi_send_)
  INTERCEPT2("mpi_recv_", libmpi_recv_)

  INTERCEPT2("mpi_sendrecv_", libmpi_sendrecv_)
  INTERCEPT2("mpi_sendrecv_replace_", libmpi_sendrecv_replace_)
  INTERCEPT2("mpi_bsend_", libmpi_bsend_)
  INTERCEPT2("mpi_ssend_", libmpi_ssend_)
  INTERCEPT2("mpi_rsend_", libmpi_rsend_)
  INTERCEPT2("mpi_isend_", libmpi_isend_)
  INTERCEPT2("mpi_ibsend_", libmpi_ibsend_)
  INTERCEPT2("mpi_issend_", libmpi_issend_)
  INTERCEPT2("mpi_irsend_", libmpi_irsend_)
  INTERCEPT2("mpi_irecv_", libmpi_irecv_)

  INTERCEPT2("mpi_wait_", libmpi_wait_)
  INTERCEPT2("mpi_waitall_", libmpi_waitall_)
  INTERCEPT2("mpi_waitany_", libmpi_waitany_)
  INTERCEPT2("mpi_waitsome_", libmpi_waitsome_)
  INTERCEPT2("mpi_test_", libmpi_test_)
  INTERCEPT2("mpi_testall_", libmpi_testall_)
  INTERCEPT2("mpi_testany_", libmpi_testany_)
  INTERCEPT2("mpi_testsome_", libmpi_testsome_)

  INTERCEPT2("mpi_probe_", libmpi_probe_)
  INTERCEPT2("mpi_iprobe_", libmpi_iprobe_)

  INTERCEPT2("mpi_get_", libmpi_get_)
  INTERCEPT2("mpi_put_", libmpi_put_)
  
  INTERCEPT2("mpi_bcast_", libmpi_bcast_)
  INTERCEPT2("mpi_gather_", libmpi_gather_)
  INTERCEPT2("mpi_gatherv_", libmpi_gatherv_)
  INTERCEPT2("mpi_scatter_", libmpi_scatter_)
  INTERCEPT2("mpi_scatterv_", libmpi_scatterv_)
  INTERCEPT2("mpi_allgather_", libmpi_allgather_)
  INTERCEPT2("mpi_allgatherv_", libmpi_allgatherv_)
  INTERCEPT2("mpi_alltoall_", libmpi_alltoall_)
  INTERCEPT2("mpi_alltoallv_", libmpi_alltoallv_)
  INTERCEPT2("mpi_reduce_", libmpi_reduce_)
  INTERCEPT2("mpi_allreduce_", libmpi_allreduce_)
  INTERCEPT2("mpi_reduce_scatter_", libmpi_reduce_scatter_)
  INTERCEPT2("mpi_scan_", libmpi_scan_)

  INTERCEPT2("mpi_comm_spawn_", libmpi_comm_spawn_)

  INTERCEPT2("mpi_send_init_", libmpi_send_init_)
  INTERCEPT2("mpi_bsend_init_", libmpi_bsend_init_)
  INTERCEPT2("mpi_ssend_init_", libmpi_ssend_init_)
  INTERCEPT2("mpi_rsend_init_", libmpi_rsend_init_)
  INTERCEPT2("mpi_recv_init_", libmpi_recv_init_)
  INTERCEPT2("mpi_start_", libmpi_start_)
  INTERCEPT2("mpi_startall_", libmpi_startall_)
END_INTERCEPT


void libinit(void) __attribute__ ((constructor));
void libinit(void)
{

   DYNAMIC_INTERCEPT_ALL();

#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#else
  /* when the application calls eztrace_start(),
   * we need to execute mpi_init_generic
   */
  eztrace_register_init_routine(&__mpi_init_generic);
#endif
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
  eztrace_stop ();
  free(proc_id);
}
