#ifndef EZTRACE_CONVERT_MPI_PERS_H
#define EZTRACE_CONVERT_MPI_PERS_H

#include <stdio.h>
#include "mpi_ev_codes.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_hierarchical_array.h"


struct pers_msg_event {
  uint64_t time;
  struct mpi_coll_msg_t *msg;
};


#define MPI_PERS_ID (MPI_PREFIX | 0x2001)
#define MPI_STATS_PERS_ID (MPI_PREFIX | 0x2100)

void init_mpi_pers_messages();

void print_pers_stats();

void __print_pers_req(FILE*stream, struct mpi_pers_req_t *req);
void __print_pers_req_recurse(FILE*stream, unsigned depth, p_eztrace_container p_cont);
void __print_all_pers_req(FILE*stream);

struct mpi_pers_req_t* __new_pers_req(enum mpi_request_type type,
				      app_ptr buffer,
				      int src,
				      int dest,
				      int len,
				      int tag,
				      app_ptr mpi_req);

struct mpi_pers_req_t*
__pers_init(uint64_t time,
	    enum mpi_request_type type,
	    app_ptr buffer,
	    int src,
	    int dest,
	    int len,
	    int tag,
	    app_ptr mpi_req);


struct mpi_pers_req_t*
__pers_start(uint64_t time,
	     int rank,
	     app_ptr mpi_req);




#endif	/* EZTRACE_CONVERT_MPI_PERS_H */
