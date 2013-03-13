/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#define LEN      1024
#define LOOPS    2
#define NB_MSG   4

static int	comm_rank	= -1;
static int	comm_size	= -1;
static char	host_name[1024]	= "";

static unsigned char **main_buffer = NULL;

/* fill the buffer */
void fill_buffer(unsigned char** buffer, int buffer_size)
{
  int i, j;
  for(i=0; i<NB_MSG; i++)
    for(j=0; j<buffer_size; j++)
      buffer[i][j] = 'a'+(j%26);
}

/* fill the buffer with 0 */
void zero_buffer(unsigned char** buffer, int buffer_size)
{
  int i, j;
  for(i=0; i<NB_MSG; i++)
    for(j=0; j<buffer_size; j++)
      buffer[i][j] = 0;
}

/* check wether the buffer contains errornous data
 * return 1 if it contains an error
 */
int check_buffer(unsigned char** buffer, int buffer_size)
{
  int i, j;
  for(i=0; i<NB_MSG; i++)
    for(j=0; j<buffer_size; j++)
      if(buffer[i][j] != 'a'+(i%26))
	return 1;
  return 0;
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
        int ping_side;
        int rank_dst;
        int start_len      = LEN;
        int iterations     = LOOPS;
	int i, j;

	MPI_Init(&argc,&argv);

        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

        if (gethostname(host_name, 1023) < 0) {
                perror("gethostname");
                exit(1);
        }

        if (comm_size < 2) {
                fprintf(stderr, "This program requires at least 2 MPI processes, aborting...\n");
                goto out;
        }

        fprintf(stdout, "(%s): My rank is %d\n", host_name, comm_rank);

        ping_side	= !(comm_rank & 1);


	main_buffer = malloc(sizeof(unsigned char*) * NB_MSG);
	for(i=0; i<NB_MSG; i++)
	  main_buffer[i] = malloc(start_len);
	fill_buffer(main_buffer, start_len);

	int size = 1024;
	//MPI_Barrier(MPI_COMM_WORLD);
	for(i = 0; i< iterations; i++) {
	  for(j=0;j<comm_rank; j++)
	    compute(100);

		if(! comm_rank) {
		  for(j=0; j<NB_MSG; j++)
		    MPI_Send(main_buffer[j], size, MPI_CHAR, (comm_rank+1)%comm_size, 0, MPI_COMM_WORLD);
		} else {
		  for(j=0; j<NB_MSG; j++)
		    MPI_Recv(main_buffer[j], size, MPI_CHAR, (comm_rank+comm_size-1)%comm_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}


		compute(100);
		if(! comm_rank) {
		  for(j=0; j<NB_MSG; j++)
		    MPI_Recv(main_buffer[j], size, MPI_CHAR, (comm_rank+comm_size-1)%comm_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		} else {
		  for(j=0; j<NB_MSG; j++)
		    MPI_Send(main_buffer[j], size, MPI_CHAR, (comm_rank+1)%comm_size, 0, MPI_COMM_WORLD);
		}

	}
	MPI_Barrier(MPI_COMM_WORLD);

 out:
	for(i=0; i<NB_MSG; i++)
	  free(main_buffer[i]);
        MPI_Finalize();

        return 0;
}
