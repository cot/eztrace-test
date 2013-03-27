#ifndef MPI_EZTRACE_H
#define MPI_EZTRACE_H

#include <stdlib.h>
#include "mpi.h"


#if MPI_VERSION >= 3
/* Use MPI 3 */
#warning MPI3 detected

/* In MPI3, the prototype of some MPI functions have change.
 * For instance, in MPI2.X, the prototype of MPI_Send was:
 * int MPI_Send(void* buffer, [...]);
 * In MPI3, MPI_Send is defined as:
 * int MPI_Send(const void* buffer, [...]);
 * In order to fix this prototype issue, let's define a CONST macro
 */
#define CONST const

#else
/* This is MPI 2.X */
#define CONST

#endif

/* maximum number of items to be allocated statically
 * if the application need more than this, a dynamic array
 * is allocated using malloc()
 */
#define MAX_REQS 128

/* allocate a number of elements using a static array if possible
 * if not possible (ie. count>MAX_REQS) use a dynamic array (ie. malloc)
 */
#define ALLOCATE_ITEMS(type, count, static_var, dyn_var)	\
  type static_var[MAX_REQS];					\
  type *dyn_var = static_var;					\
  if((count) > MAX_REQS)					\
    dyn_var = (type*) malloc(sizeof(type)*(count))

/* Free an array created by ALLOCATE_ITEMS */
#define FREE_ITEMS(count, dyn_var)		\
  if((count) > MAX_REQS)			\
    free(dyn_var)

/* convert a C request (ie. a pointer to a MPI_Request structure)
 * to an integer
 */
//#define EZTRACE_REQ(r) MPI_Request_c2f(*(r))
int _UNUSED_MPI ;
#define MAX_LEVEL_MPI 2

/* pointers to actual MPI functions (C version)  */
extern int ( *libMPI_Init) (int *, char ***);
extern int ( *libMPI_Init_thread) (int *, char ***, int, int*);
extern int ( *libMPI_Comm_size) (MPI_Comm, int *);
extern int ( *libMPI_Comm_rank) (MPI_Comm, int *);
extern int ( *libMPI_Finalize) (void);
extern int ( *libMPI_Initialized) (int *);
extern int ( *libMPI_Abort) (MPI_Comm, int);

extern int ( *libMPI_Send) (CONST void *buf, int count, MPI_Datatype datatype,int dest, int tag,MPI_Comm comm);
extern int ( *libMPI_Recv) (void *buf, int count, MPI_Datatype datatype,int source, int tag, MPI_Comm comm, MPI_Status *status);

extern int ( *libMPI_Bsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
extern int ( *libMPI_Ssend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
extern int ( *libMPI_Rsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
extern int ( *libMPI_Isend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Ibsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Issend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Irsend) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Irecv) (void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);

extern int ( *libMPI_Sendrecv) (CONST void *, int, MPI_Datatype,int, int, void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *);
extern int ( *libMPI_Sendrecv_replace) (void*, int, MPI_Datatype, int, int, int, int, MPI_Comm, MPI_Status *);

extern int ( *libMPI_Send_init) (CONST void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Bsend_init) (CONST void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Ssend_init) (CONST void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Rsend_init) (CONST void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Recv_init) (void*, int, MPI_Datatype, int,int, MPI_Comm, MPI_Request *);
extern int ( *libMPI_Start) (MPI_Request *);
extern int ( *libMPI_Startall) (int, MPI_Request *);

extern int ( *libMPI_Wait) (MPI_Request *, MPI_Status *);
extern int ( *libMPI_Test) (MPI_Request *, int *, MPI_Status *);
extern int ( *libMPI_Waitany) (int, MPI_Request *, int *, MPI_Status *);
extern int ( *libMPI_Testany) (int, MPI_Request *, int *, int *, MPI_Status *);
extern int ( *libMPI_Waitall) (int, MPI_Request *, MPI_Status *);
extern int ( *libMPI_Testall) (int, MPI_Request *, int *, MPI_Status *);
extern int ( *libMPI_Waitsome) (int, MPI_Request *, int *, int *, MPI_Status *);
extern int ( *libMPI_Testsome) (int, MPI_Request *, int *, int *, MPI_Status *);

extern int ( *libMPI_Probe)( int source, int tag, MPI_Comm comm, MPI_Status *status );
extern int ( *libMPI_Iprobe)( int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status );

extern int ( *libMPI_Barrier) (MPI_Comm );
extern int ( *libMPI_Bcast) (void*, int, MPI_Datatype, int, MPI_Comm );
extern int ( *libMPI_Gather) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm);
extern int ( *libMPI_Gatherv) (CONST void* , int, MPI_Datatype, void*, CONST int *, CONST int *, MPI_Datatype, int, MPI_Comm);
extern int ( *libMPI_Scatter) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm);
extern int ( *libMPI_Scatterv) (CONST void* , CONST int *, CONST int *,  MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm);
extern int ( *libMPI_Allgather) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, MPI_Comm);
extern int ( *libMPI_Allgatherv) (CONST void* , int, MPI_Datatype, void*, CONST int *, CONST int *, MPI_Datatype, MPI_Comm);
extern int ( *libMPI_Alltoall) (CONST void* , int, MPI_Datatype, void*, int, MPI_Datatype, MPI_Comm);
extern int ( *libMPI_Alltoallv) (CONST void*, CONST int *, CONST int *, MPI_Datatype, void*, CONST int *, CONST int *, MPI_Datatype, MPI_Comm);
extern int ( *libMPI_Reduce) (CONST void* , void*, int, MPI_Datatype, MPI_Op, int, MPI_Comm);
extern int ( *libMPI_Allreduce) (CONST void* , void*, int, MPI_Datatype, MPI_Op, MPI_Comm);
extern int ( *libMPI_Reduce_scatter) (CONST void* , void*, CONST int *, MPI_Datatype, MPI_Op, MPI_Comm);
extern int ( *libMPI_Scan) (CONST void* , void*, int, MPI_Datatype, MPI_Op, MPI_Comm );

extern int ( *libMPI_Get) (void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
			   MPI_Win);
extern int ( *libMPI_Put) (CONST void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
			   MPI_Win);

extern int ( *libMPI_Comm_spawn)(CONST char *command,
				 char *argv[],
				 int maxprocs,
				 MPI_Info info,
				 int root,
				 MPI_Comm comm,
				 MPI_Comm *intercomm,
				 int array_of_errcodes[]);



/* fortran bindings */
extern void (*libmpi_init_)(int*e);
extern void (*libmpi_init_thread_)(int*, int*, int*);
extern void (*libmpi_finalize_)(int*);
extern void (*libmpi_barrier_)(MPI_Comm*, int*);
extern void (*libmpi_comm_size_)(MPI_Comm*, int*, int*);
extern void (*libmpi_comm_rank_)(MPI_Comm*, int*, int*);

extern void (*libmpi_send_)(void*, int*, MPI_Datatype*, int*, int*, int*);
extern void (*libmpi_recv_)(void*, int*, MPI_Datatype*, int*, int *, MPI_Status *, int*);

extern void ( *libmpi_sendrecv_) (void *, int, MPI_Datatype,int, int, void *, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status *, int*);
extern void ( *libmpi_sendrecv_replace_) (void*, int, MPI_Datatype, int, int, int, int, MPI_Comm, MPI_Status *, int*);

extern void (*libmpi_bsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
extern void (*libmpi_ssend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
extern void (*libmpi_rsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
extern void (*libmpi_isend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request*, int*);
extern void (*libmpi_ibsend_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request*, int*);
extern void (*libmpi_issend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
extern void (*libmpi_irsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
extern void (*libmpi_irecv_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *,  int*);

extern void (*libmpi_wait_)(MPI_Request*, MPI_Status*, int*);
extern void (*libmpi_test_)(MPI_Request*, int*, MPI_Status*, int*);
extern void (*libmpi_waitany_) (int*, MPI_Request *, int *, MPI_Status *, int*);
extern void (*libmpi_testany_) (int*, MPI_Request *, int *, int *, MPI_Status *, int*);
extern void (*libmpi_waitall_) (int*, MPI_Request *, MPI_Status *, int*);
extern void (*libmpi_testall_) (int*, MPI_Request *, int *, MPI_Status *, int*);
extern void (*libmpi_waitsome_) (int*, MPI_Request *, int *, int *, MPI_Status *, int*);
extern void (*libmpi_testsome_) (int*, MPI_Request *, int *, int *, MPI_Status *, int*);

extern void ( *libmpi_probe_)( int* source, int* tag, MPI_Comm* comm, MPI_Status *status, int* err );
extern void ( *libmpi_iprobe_)( int* source, int* tag, MPI_Comm* comm, int *flag, MPI_Status *status, int* err );

extern void (*libmpi_get_)(void *, int*, MPI_Datatype*, int*, MPI_Aint*, int*, MPI_Datatype*, MPI_Win*, int*);
extern void (*libmpi_put_)(void *, int*, MPI_Datatype*, int*, MPI_Aint*, int*, MPI_Datatype*, MPI_Win*, int*);

extern void (*libmpi_bcast_)(void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
extern void (*libmpi_gather_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
extern void (*libmpi_gatherv_)(void*, int*, MPI_Datatype*, void*, int*, int*, MPI_Datatype*, int*, MPI_Comm*);
extern void (*libmpi_scatter_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
extern void (*libmpi_scatterv_)(void*, int*, int*,  MPI_Datatype*, void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
extern void (*libmpi_allgather_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, MPI_Comm*, int*);
extern void (*libmpi_allgatherv_)(void*, int*, MPI_Datatype*, void*, int*, int*, MPI_Datatype*, MPI_Comm*);
extern void (*libmpi_alltoall_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*, MPI_Comm*, int*);
extern void (*libmpi_alltoallv_)(void*, int*, int*, MPI_Datatype*, void*, int*, int*, MPI_Datatype*, MPI_Comm*, int*);
extern void (*libmpi_reduce_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, int*, MPI_Comm*, int*);
extern void (*libmpi_allreduce_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*, int*);
extern void (*libmpi_reduce_scatter_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*, int*);
extern void (*libmpi_scan_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*, int*);

extern void (*libmpi_comm_spawn_)(char *command, char **argv, int *maxprocs,
			   MPI_Info *info, int *root, MPI_Comm *comm,
			   MPI_Comm *intercomm, int *array_of_errcodes, int*error);

extern void ( *libmpi_send_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
extern void ( *libmpi_bsend_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
extern void ( *libmpi_ssend_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
extern void ( *libmpi_rsend_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
extern void ( *libmpi_recv_init_) (void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, MPI_Request *, int*);
extern void ( *libmpi_start_) (MPI_Request *, int*);
extern void ( *libmpi_startall_) (int*, MPI_Request *, int*);

#endif	/* MPI_EZTRACE_H */
