#include "eztrace.h"

#include "alltoall_ev_codes.h"

int (*libMPI_Alltoall) (void * sendbuf, int sendcount, void* sendtype, void* recvbuf, int recvcount, void* recvtype, void* comm);



int MPI_Alltoall (void * sendbuf, int sendcount, void* sendtype, void* recvbuf, int recvcount, void* recvtype, void* comm) {
	int ret = libMPI_Alltoall (sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
	return ret;
}



START_INTERCEPT
  INTERCEPT2("MPI_Alltoall", libMPI_Alltoall)


END_INTERCEPT

static void __alltoall_init (void) __attribute__ ((constructor));
/* Initialize the current library */
static void
__alltoall_init (void)
{
  DYNAMIC_INTERCEPT_ALL();

  /* start event recording */
#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#endif
}

static void __alltoall_conclude (void) __attribute__ ((destructor));
static void
__alltoall_conclude (void)
{
  /* stop event recording */
  eztrace_stop ();
}
