#ifndef EZTRACE_CONVERT_MPI_P2P_H
#define EZTRACE_CONVERT_MPI_P2P_H

#include <stdio.h>
#include "mpi_ev_codes.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_hierarchical_array.h"


struct p2p_msg_event {
  uint64_t time;
  struct mpi_p2p_msg_t *msg;
};

#define MPI_P2P_ID (MPI_PREFIX | 0x0001)
#define MPI_P2P_ISEND_ID (MPI_PREFIX | 0x0010)
#define MPI_P2P_SWAIT_ID (MPI_PREFIX | 0x0011)
#define MPI_P2P_IRECV_ID (MPI_PREFIX | 0x0012)
#define MPI_P2P_RWAIT_ID (MPI_PREFIX | 0x0013)
#define MPI_STATS_P2P_ID (MPI_PREFIX | 0x0100)

#define MPI_REQUEST_ID (MPI_PREFIX | 0x0020)


void init_mpi_p2p_messages();

void print_p2p_stats();

void __print_freq();

void __print_p2p_message(FILE*stream, struct mpi_p2p_msg_t *msg);
void __print_p2p_messages_recurse(FILE*stream, unsigned depth, p_eztrace_container p_cont);
void __print_p2p_messages(FILE*stream);

struct mpi_p2p_msg_t* __new_p2p_message(char* id,
					int src,
					int dest,
					int len,
					uint32_t tag,
					int unexp,
					char* link_value,
					const char* sender_thread_id,
					const struct mpi_request* sender_request,
					const char* recver_thread_id,
					const struct mpi_request* recver_request);


struct mpi_request* __mpi_new_mpi_request(int rank,
					  app_ptr mpi_req,
					  enum mpi_request_type req_type);

struct mpi_request*
__mpi_find_mpi_req(int rank,
		   app_ptr mpi_req,
		   enum mpi_request_type req_type);

struct mpi_p2p_msg_t*
__mpi_find_p2p_message_by_mpi_req(int rank, const struct mpi_request* request);

struct mpi_p2p_msg_t*
__mpi_find_p2p_message(int src __attribute__((unused)),
		   int dest,
		   int len __attribute__((unused)),
		   uint32_t tag,
		   int time_id);

struct mpi_p2p_msg_t*
__start_recv_message(uint64_t start_time, int src, int dest, int len, uint32_t tag,
		     const char* thread_id, struct mpi_request* mpi_req);

struct mpi_p2p_msg_t*
__stop_recv_message(uint64_t stop_time,
		    int src,
		    int dest,
		    int len,
		    uint32_t tag,
		    const char* thread_id __attribute__((unused)),
		    struct mpi_request* mpi_req);

struct mpi_p2p_msg_t*
__start_send_message(uint64_t start_time,
		     int src,
		     int dest,
		     int len,
		     uint32_t tag,
		     const char* thread_id,
		     struct mpi_request* mpi_req);

struct mpi_p2p_msg_t*
__stop_send_message(uint64_t stop_time,
		    int src,
		    int dest,
		    int len,
		    uint32_t tag,
		    const char* thread_id __attribute__((unused)),
		    struct mpi_request* mpi_req);


#endif	/* EZTRACE_CONVERT_MPI_P2P_H */
