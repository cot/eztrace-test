#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char **argv )
{
    int rank, size;
    int errors=0;
    MPI_Comm comm = MPI_COMM_WORLD; 
    int sendarray[100]; 
    int *rbuf; 

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    /* Exactly 2 processes can participate */
    if ( size < 2 ) {
        fprintf( stderr, "Number of processors must be at least 2\n");fflush(stderr);
        MPI_Abort( MPI_COMM_WORLD, 1 );
    }

    rbuf = (int *)malloc(size*100*sizeof(int)); 
    MPI_Allgather( sendarray, 100, MPI_INT, rbuf, 100, MPI_INT, comm); 

    MPI_Finalize();
    if (errors)
    {
        printf( "[%d] done with ERRORS(%d)!\n", rank, errors );fflush(stdout);
    }
    return errors;
}
