#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int numtasks, rank;
  int rank_dst, ping_side;

  // Initialise MPI
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  if (numtasks != 2) {
    printf("Need 2 processes\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
  }

  ping_side = !(rank & 1);
  rank_dst = ping_side?(rank | 1) : (rank & ~1);

  if (ping_side) {
    int x=42, y;
    MPI_Request send_request;
    MPI_Request recv_request;

    MPI_Send_init(&x, 1, MPI_INT, rank_dst, 1, MPI_COMM_WORLD, &send_request);
    MPI_Start(&send_request);
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);

    MPI_Start(&send_request);
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);

    MPI_Recv_init(&y, 1, MPI_INT, rank_dst, 1, MPI_COMM_WORLD, &recv_request);
    MPI_Start(&recv_request);
    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

    if (y == 42) 
      printf("success\n"); 
    else
      printf("failure\n");

    MPI_Start(&recv_request);
    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

    if (y == 42) 
      printf("success\n"); 
    else
      printf("failure\n");
  }
  else {
    int x, y;
    MPI_Recv(&x, 1, MPI_INT, rank_dst, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&y, 1, MPI_INT, rank_dst, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Send(&y, 1, MPI_INT, rank_dst, 1, MPI_COMM_WORLD);
    MPI_Send(&y, 1, MPI_INT, rank_dst, 1, MPI_COMM_WORLD);

    if (x == 42) 
      printf("success\n"); 
    else
      printf("failure\n");
    if (y == 42) 
      printf("success\n"); 
    else
      printf("failure\n");
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  exit(0);
}
