/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#define LEN      1024
#define LOOPS    10

static int	comm_rank	= -1;
static int	comm_size	= -1;
static char	host_name[1024]	= "";

static unsigned char *main_buffer = NULL;

/* fill the buffer */
void fill_buffer(unsigned char* buffer, int buffer_size)
{
	int i;
	for(i=0; i<buffer_size; i++)
		buffer[i] = 'a'+(i%26);
}

/* fill the buffer with 0 */
void zero_buffer(unsigned char* buffer, int buffer_size)
{
	int i;
	for(i=0; i<buffer_size; i++)
		buffer[i] = 0;
}

/* check wether the buffer contains errornous data
 * return 1 if it contains an error
 */
int check_buffer(unsigned char* buffer, int buffer_size)
{
	int i;
	for(i=0; i<buffer_size; i++)
		if(buffer[i] != 'a'+(i%26))
			return 1;
	return 0;
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
        int start_len      = LEN;
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

        ping_side	= !(comm_rank & 1);
        rank_dst	= ping_side?(comm_rank | 1):(comm_rank & ~1);

        main_buffer = malloc(start_len);
	fill_buffer(main_buffer, start_len);

        MPI_Barrier(MPI_COMM_WORLD);

        /* Test */
        if( ping_side) {
                int size = start_len;
		int nb_samples;

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
			MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
			MPI_Recv(main_buffer, size, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC passed\n", ping_side);


		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
			MPI_Recv(main_buffer, size, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC+ANYTAG failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC+ANYTAG passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
			MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYTAG failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYTAG passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			fill_buffer(main_buffer, size);
			MPI_Ssend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
			MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Ssend/MPI_Recv failed !\n", ping_side);
				goto out;
			}

		}
		fprintf(stderr, "[%d] MPI_Ssend/MPI_Recv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			fill_buffer(main_buffer, size);
			MPI_Rsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
			MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Rsend/MPI_Recv failed !\n", ping_side);
				goto out;
			}

		}
		fprintf(stderr, "[%d] MPI_Rsend/MPI_Recv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		/* non blocking functions */
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			fill_buffer(main_buffer, size);
			MPI_Isend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			compute(100);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			zero_buffer(main_buffer, size);
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Isend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Isend/MPI_Irecv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			fill_buffer(main_buffer, size);
			MPI_Issend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			compute(100);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			zero_buffer(main_buffer, size);
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Issend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Issend/MPI_Irecv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			fill_buffer(main_buffer, size);
			MPI_Irsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			compute(100);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			zero_buffer(main_buffer, size);
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);

		/* MPI_Bsend */
		int bufsize = 1024 + MPI_BSEND_OVERHEAD;
		char *buf = malloc(bufsize);
		MPI_Buffer_attach( buf, bufsize);

		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			fill_buffer(main_buffer, size);
			MPI_Bsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}
		}
		fprintf(stderr, "[%d] MPI_Bsend/MPI_Irecv passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);


		/* MPI_Ibsend */
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			fill_buffer(main_buffer, size);
			MPI_Bsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			compute(100);
			zero_buffer(main_buffer, size);
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}
		}
		MPI_Buffer_detach( &buf, &bufsize );
		fprintf(stderr, "[%d] MPI_Ibsend/MPI_Irecv passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);


		/* MPI_Waitall */
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req[4];
			int vars[4];
			int i;
			for(i=0; i<4; i++) {
				vars[i] = i*i;
				MPI_Isend(&vars[i], 1, MPI_INTEGER, rank_dst, 0, MPI_COMM_WORLD, &req[i]);
			}
			MPI_Waitall(4, req, MPI_STATUS_IGNORE);
			compute(100);

			for(i=0; i<4; i++) {
				vars[i] = -1;
				MPI_Irecv(&vars[i], 1, MPI_INTEGER, rank_dst, 0, MPI_COMM_WORLD, &req[i]);
			}
			MPI_Waitall(4, req, MPI_STATUS_IGNORE);

			for(i=0; i<4; i++) {
				if(vars[i] != i*i){
					fprintf(stderr, "[%d] MPI_Waitall failed !\n", ping_side);
					goto out;
				}
			}
		}
		fprintf(stderr, "[%d] MPI_Waitall passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);

	} else {
                int size = start_len;
		int nb_samples;

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv passed\n", ping_side);


		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Recv(main_buffer, size, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
		  MPI_Recv(main_buffer, size, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC+ANYTAG failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYSRC+ANYTAG passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
		  MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYTAG failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Send(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Send/MPI_Recv ANYTAG passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD,  MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Ssend/MPI_Recv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Ssend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Ssend/MPI_Recv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Recv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Rsend/MPI_Recv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Rsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Rsend/MPI_Recv passed\n", ping_side);

		/* non blocking functions */
		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Isend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Isend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Isend/MPI_Irecv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Isend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Issend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Issend/MPI_Irecv passed\n", ping_side);

		MPI_Barrier(MPI_COMM_WORLD);
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Irsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);

		/* MPI_Bsend */
		int bufsize = 1024 + MPI_BSEND_OVERHEAD;
		char *buf = malloc(bufsize);
		MPI_Buffer_attach( buf, bufsize);

		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Bsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD);
			zero_buffer(main_buffer, size);
		}
		fprintf(stderr, "[%d] MPI_Bsend/MPI_Irecv passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);


		/* MPI_Ibsend */
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req;
			MPI_Irecv(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			if(check_buffer(main_buffer, size)) {
				fprintf(stderr, "[%d] MPI_Irsend/MPI_Irecv failed !\n", ping_side);
				goto out;
			}

			fill_buffer(main_buffer, size);
			MPI_Ibsend(main_buffer, size, MPI_CHAR, rank_dst, 0, MPI_COMM_WORLD, &req);
			MPI_Wait(&req, MPI_STATUS_IGNORE);
			zero_buffer(main_buffer, size);
		}
		MPI_Buffer_detach( &buf, &bufsize );
		fprintf(stderr, "[%d] MPI_Ibsend/MPI_Irecv passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);


		/* MPI_Ibsend */
		for (nb_samples=0; nb_samples < iterations; nb_samples++) {
			MPI_Request req[4];
			int vars[4];
			int i;
			for(i=0; i<4; i++) {
				vars[i] = -1;
				MPI_Irecv(&vars[i], 1, MPI_INTEGER, rank_dst, 0, MPI_COMM_WORLD, &req[i]);
			}
			MPI_Waitall(4, req, MPI_STATUS_IGNORE);
			compute(100);

			for(i=0; i<4; i++) {
				MPI_Isend(&vars[i], 1, MPI_INTEGER, rank_dst, 0, MPI_COMM_WORLD, &req[i]);
			}

			MPI_Waitall(4, req, MPI_STATUS_IGNORE);

			for(i=0; i<4; i++) {
				if(vars[i] != i*i){
					fprintf(stderr, "[%d] MPI_Waitall failed !\n", ping_side);
					goto out;
				}
			}
		}
		fprintf(stderr, "[%d] MPI_Waitall passed\n", ping_side);
		MPI_Barrier(MPI_COMM_WORLD);
	}
	free(main_buffer);
 out:
        MPI_Finalize();

        return 0;
}
