/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

//#define LEN      (1024*1024*100)
#define LEN      (10*1024*1024)
#define LOOPS    5

static int	comm_rank	= -1;
static int	comm_size	= -1;
static char	host_name[1024]	= "";

static unsigned char *main_buffer = NULL;

/* fill the buffer */
void fill_buffer(double* buffer, int buffer_size)
{
	int i;
	for(i=0; i<buffer_size; i++)
	  buffer[i] = (double)i;
}

/* fill the buffer with 0 */
void zero_buffer(double* buffer, int buffer_size)
{
	int i;
	for(i=0; i<buffer_size; i++)
		buffer[i] = 0;
}

/* 'Compute' for a fixed duration */
void compute(unsigned usec)
{
	double t1, t2;
	t1 = MPI_Wtime();
	do {
		t2 = MPI_Wtime();
	} while(((t2-t1) * 1e6) < (double) usec);
}

int
main(int    argc,
     char **argv)
{
        int ping_side;
        int rank_dst;

	int required = MPI_THREAD_SERIALIZED;
	int provided;
	  MPI_Init_thread(&argc,&argv, required, &provided);
	if(required != provided) {
	  fprintf(stderr, "MPI does not support thread safety level %d\n", required);
	  return 1;
	}

        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

        if (gethostname(host_name, 1023) < 0) {
                perror("gethostname");
                exit(1);
        }

        if (comm_size != 2) {
                fprintf(stderr, "This program requires 2 MPI processes, aborting...\n");
                goto out;
        }

        fprintf(stdout, "(%s): My rank is %d\n", host_name, comm_rank);

        ping_side	= !(comm_rank & 1);
        rank_dst	= ping_side?(comm_rank | 1):(comm_rank & ~1);

	int i;
        int nb_iter = LOOPS;
	int size = LEN;
	int iter = 0;
        double *A = malloc(size * sizeof(double));
        double *B = malloc(size * sizeof(double));
	fill_buffer(A, size);
	zero_buffer(B, size);

	MPI_Request req[2];
	MPI_Barrier(MPI_COMM_WORLD);
	for(iter = 0; iter<nb_iter; iter++) {
	  fprintf(stderr, "[%d] loop %d\n", comm_rank, iter);
#pragma omp parallel for
	  for(i=0; i<size; i++) {
	    A[i] = B[i] + A[i];
	  }
	  MPI_Isend(A, size, MPI_DOUBLE, rank_dst, 0, MPI_COMM_WORLD, &req[0]);
	  MPI_Recv(B, size, MPI_DOUBLE, rank_dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  MPI_Wait(&req[0], MPI_STATUS_IGNORE);
	}		  
	MPI_Barrier(MPI_COMM_WORLD);
	
	free(A);
	free(B);

 out:
        MPI_Finalize();

        return 0;
}
