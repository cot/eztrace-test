#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define NPROCS 4

int main(int argc, char *argv[]) {
  int        rank, new_rank, sendbuf, recvbuf, numtasks, ranks1[2]={0,1}, ranks2[2]={2,3};
  MPI_Group  orig_group, new_group;
  MPI_Comm   new_comm;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  if (numtasks != NPROCS) {
    printf("Must specify MP_PROCS= %d. Terminating.\n",NPROCS);
    MPI_Finalize();
    exit(0);
  }

  sendbuf = rank;

  /* Extract the original group handle */
  MPI_Comm_group(MPI_COMM_WORLD, &orig_group);

  /* Divide tasks into two distinct groups based upon rank */
  if (rank < NPROCS/2) {
    MPI_Group_incl(orig_group, NPROCS/2, ranks1, &new_group);
  }
  else {
    MPI_Group_incl(orig_group, NPROCS/2, ranks2, &new_group);
  }

  /* Create new new communicator and then perform collective communications */
  MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);

  int i;

  /* test blocking communications */
  for(i=0; i<NPROCS/2; i++) {
    MPI_Send(&sendbuf, 1, MPI_INT, i, 0, new_comm);
  }

  for(i=0; i<NPROCS/2; i++) {
    MPI_Recv(&recvbuf, 1, MPI_INT, i, 0, new_comm, MPI_STATUS_IGNORE);
  }

  /* test non-blocking communications */
  MPI_Request reqs[NPROCS/2];
  for(i=0; i<NPROCS/2; i++) {
    MPI_Isend(&sendbuf, 1, MPI_INT, i, 0, new_comm, &reqs[i]);
  }
  MPI_Waitall(NPROCS/2, reqs, MPI_STATUSES_IGNORE);

  for(i=0; i<NPROCS/2; i++) {
    MPI_Irecv(&recvbuf, 1, MPI_INT, i, 0, new_comm, &reqs[i]);
  }
  MPI_Waitall(NPROCS/2, reqs, MPI_STATUSES_IGNORE);

  /* test persistant requests */
  for(i=0; i<NPROCS/2; i++) {
    MPI_Send_init(&sendbuf, 1, MPI_INT, i, 0, new_comm, &reqs[i]);
  }

  MPI_Startall(NPROCS/2, reqs);
  MPI_Waitall(NPROCS/2, reqs, MPI_STATUSES_IGNORE);

  MPI_Startall(NPROCS/2, reqs);
  MPI_Waitall(NPROCS/2, reqs, MPI_STATUSES_IGNORE);

  for(i=0; i<NPROCS/2; i++) {
    MPI_Recv_init(&recvbuf, 1, MPI_INT, i, 0, new_comm, &reqs[i]);
  }

  MPI_Startall(NPROCS/2, reqs);
  MPI_Waitall(NPROCS/2, reqs, MPI_STATUSES_IGNORE);

  MPI_Startall(NPROCS/2, reqs);
  MPI_Waitall(NPROCS/2, reqs, MPI_STATUSES_IGNORE);

  MPI_Group_rank (new_group, &new_rank);

  printf("rank= %d newrank= %d recvbuf= %d\n",rank,new_rank,recvbuf);

  sleep(1);

  /* test blocking communications for MPI_COMM_WORLD */
  for(i=0; i<NPROCS; i++) {
    MPI_Send(&sendbuf, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
  }

  for(i=0; i<NPROCS; i++) {
    MPI_Recv(&recvbuf, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  MPI_Finalize();
  return 0;
}
