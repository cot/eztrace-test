
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_SPAWNS 2

/* 'Compute' for a fixed duration */
void compute(unsigned usec)
{
  double t1, t2;
  t1 = MPI_Wtime();
  do {
    t2 = MPI_Wtime();
  } while(((t2-t1) * 1e6) < (double) usec);
}


int main( int argc, char *argv[] )
{
    int np = NUM_SPAWNS;
    int errcodes[NUM_SPAWNS];
    MPI_Comm parentcomm, intercomm;

    MPI_Init( &argc, &argv );
    MPI_Comm_get_parent( &parentcomm );

    compute(100000);
    if (parentcomm == MPI_COMM_NULL)
    {
      MPI_Comm_spawn( "./mpi_spawn", MPI_ARGV_NULL, np, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &intercomm, errcodes );
      printf("I'm the parent.\n");
      compute(1000000);
    }
    else
    {
      printf("I'm the spawned.\n");
    }

    fflush(stdout);
    MPI_Finalize();
    return 0;
}
