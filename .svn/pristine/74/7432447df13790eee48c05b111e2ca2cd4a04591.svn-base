/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#define LEN      16
#define LOOPS    10000
#define WARMUP   100

static unsigned char *main_buffer = NULL;

void SEND(char* buffer, int len, int dest, int tag)
{
	MPI_Send(buffer, len, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
}

void RECV(char* buffer, int len, int src, int tag)
{
	MPI_Recv(buffer, len, MPI_CHAR, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void fill_buffer(char *buffer, int len)
{
	int i;
	for(i=0;i<len; i++) {
		buffer[i]='a'+(i%26);
	}
}

void compute(unsigned usec)
{
	double t1, t2;
	t1 = MPI_Wtime();
	do{
		t2 = MPI_Wtime();
	} while ( (t2-t1)*1e6 <usec);
}

int
main(int    argc,
     char **argv)
{
	int	comm_rank	= -1;
	int	comm_size	= -1;
	char	host_name[1024]	= "";
        int dest;
        int len            = LEN;
        int iterations     = LOOPS;

	MPI_Init(&argc,&argv);

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

	dest = (comm_rank+1)%2;

        main_buffer = malloc(len);
	fill_buffer(main_buffer, len);

	int i;

	for(i = 0; i< WARMUP; i++) {
		if(! comm_rank) {
			SEND(main_buffer, len, dest, 0);
			RECV(main_buffer, len, dest, 0);
		} else {
			RECV(main_buffer, len, dest, 0);
			SEND(main_buffer, len, dest, 0);
		}
	}

	double t1, t2;
	t1 = MPI_Wtime();
        MPI_Barrier(MPI_COMM_WORLD);
	for(i = 0; i< iterations; i++) {
		if(! comm_rank) {
			SEND(main_buffer, len, dest, 0);
			RECV(main_buffer, len, dest, 0);
		} else {
			RECV(main_buffer, len, dest, 0);
			SEND(main_buffer, len, dest, 0);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	t2 = MPI_Wtime();

	if(! comm_rank) {
		double latency = 1e6*(t2-t1)/(2*iterations);
		printf("%d\t%d\t%lf\n",iterations, len, latency);
	}
 out:
	free(main_buffer);
        MPI_Finalize();

        return 0;
}
