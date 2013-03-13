#ifndef EZTRACE_CONVERT_MPI_COLL_H
#define EZTRACE_CONVERT_MPI_COLL_H

#include <stdio.h>
#include "mpi_ev_codes.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_hierarchical_array.h"


struct coll_msg_event {
  uint64_t time;
  struct mpi_coll_msg_t *msg;
};


#define MPI_COLL_ID (MPI_PREFIX | 0x1001)
#define MPI_COLL_ENTER_ID (MPI_PREFIX | 0x1010)
#define MPI_COLL_LEAVE_ID (MPI_PREFIX | 0x1011)

#define MPI_STATS_COLL_ID (MPI_PREFIX | 0x1100)

void init_mpi_coll_messages();

void print_coll_stats();

void __print_coll_message(FILE*stream, struct mpi_coll_msg_t *msg);
void __print_coll_messages_recurse(FILE*stream, unsigned depth, p_eztrace_container p_cont);
void __print_coll_messages(FILE*stream);

struct mpi_coll_msg_t*
__enter_coll(uint64_t time,
	     enum coll_type_t type,
	     int comm_size,
	     int my_rank,
	     int len,
	     char* thread_id);

struct mpi_coll_msg_t*
__leave_coll(uint64_t time,
	     enum coll_type_t type,
	     int comm_size,
	     int my_rank,
	     int len,
	     char* thread_id);


#endif	/* EZTRACE_CONVERT_MPI_COLL_H */
