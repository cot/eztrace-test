#include "mpi.h"

#include <stdio.h>
#include <stdlib.h>

#define _size 1024*1024

int herbertvonkrugell(int toto) {
int letoto = 2 * toto;
sleep(1);
return letoto;
}

void main (int argc, char *argv[]) {
	int myproc, nprocs, i, iter, size;
	double t0, t1, time;
	double *a, *b, *c;
	MPI_Request request;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myproc);

	if(myproc == 0) 
		printf("l'objectif du prog est de tester le filtrage par eztrace de MPI_Alltoall et/ou MPI_Allgather\n");

	if(!argv[1]) iter = 10 ;
	else iter = atoi(argv[1]);

	if(!argv[2]) size = _size ;
	else size = atoi(argv[2]);

	a = (double*) malloc (size * sizeof (double));
	b = (double*) calloc (nprocs * size , sizeof (double));
	c = (double*) calloc (nprocs * size , sizeof (double));

	printf("Hello from %d of %d\n", myproc, nprocs);

	for (i = 0; i < size ; i++) 
		a[i] = (double) i;

/*####################################################################################*/
/* Test du Allgather */
	MPI_Barrier(MPI_COMM_WORLD);
	t0 = MPI_Wtime();

	for(i=0;i<iter;i++) 
			MPI_Allgather(a,size,MPI_DOUBLE,b,size,MPI_DOUBLE,MPI_COMM_WORLD);
	
	t1 = MPI_Wtime();
	MPI_Barrier(MPI_COMM_WORLD);
	time = 1.e6 * (t1 - t0);

	for(i=0;i<size;i++)
		if(b[i] != i) perror("cabron !");
		else if((myproc==0) && (i==size-1)) printf("b[size - 1] = %g \n",b[size-1]);

	if (myproc==0) printf(" %7d bytes took %9.0f usec (%8.3f MB/sec)\n",size, time, size / time);

/* Fin du Allgather */
/*####################################################################################*/
	int bob = herbertvonkrugell(2);
/*####################################################################################*/
/* Test du Alltoall */
        MPI_Barrier(MPI_COMM_WORLD);
        t0 = MPI_Wtime();

        for(i=0;i<iter;i++) {
		MPI_Alltoall(b,size,MPI_DOUBLE,c,size,MPI_DOUBLE,MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);
        }

        t1 = MPI_Wtime();
        MPI_Barrier(MPI_COMM_WORLD);
        time = 1.e6 * (t1 - t0);

        for(i=0;i<size;i++)
                if(b[i] != i) perror("cabron !");
                else if((myproc==0) && (i==size-1)) printf("c[size - 1] = %g \n",c[size-1]);

        if (myproc==0) printf(" %7d bytes took %9.0f usec (%8.3f MB/sec)\n",size, time, size / time);

/* Fin du Alltoall */
/*####################################################################################*/

	free(a);
	free(b);
	free(c);

	MPI_Finalize();
}
