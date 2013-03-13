#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include "mpi_ev_codes.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_hierarchical_array.h"
#include "eztrace_convert_mpi_p2p.h"
#include "eztrace_convert_mpi_pers.h"


void init_mpi_pers_messages()
{
  hierarchical_array_attach(MPI_PERS_ID, sizeof(struct mpi_coll_msg_t));
}

void print_pers_stats();

void __print_pers_req(FILE*stream, struct mpi_pers_req_t *msg)
{
}

void __print_pers_req_recurse(FILE*stream, unsigned depth, p_eztrace_container p_cont);
void __print_all_pers_req(FILE*stream)
{
  struct hierarchical_array* array = hierarchical_array_find(MPI_PERS_ID, NULL);
  struct mpi_pers_req_t* msg = NULL;
  unsigned index;

  for(index = 0; index< (array->nb_items); index++) {
    msg = ITH_ITEM(index, array);
    __print_pers_req(stream, msg);
  }
}

struct mpi_pers_req_t* __new_pers_req(enum mpi_request_type type,
				      app_ptr buffer,
				      int src,
				      int dest,
				      int len,
				      int tag,
				      app_ptr mpi_req)
{
  struct mpi_pers_req_t* req = NULL;
  if(type == mpi_req_recv) {
    req = hierarchical_array_new_item(GET_PROCESS_CONTAINER(dest), MPI_PERS_ID);
    req->mpi_req = __mpi_new_mpi_request(dest, mpi_req, type);
  } else {
    req = hierarchical_array_new_item(GET_PROCESS_CONTAINER(src), MPI_PERS_ID);
    req->mpi_req = __mpi_new_mpi_request(src, mpi_req, type);
  }

  req->type = type;
  req->buffer = buffer;
  req->len = len;
  req->src = src;
  req->dest = dest;
  req->tag = tag;

  return req;
}

struct mpi_pers_req_t*
__pers_init(uint64_t __attribute__((unused)) time,
	    enum mpi_request_type type,
	    app_ptr buffer,
	    int src,
	    int dest,
	    int len,
	    int tag,
	    app_ptr mpi_req)
{
  return __new_pers_req(type, buffer, src, dest, len, tag, mpi_req);
}


struct mpi_pers_req_t*
__mpi_find_pers_req(p_eztrace_container p_cont, struct mpi_request* mpi_req)
{
  struct hierarchical_array* array = hierarchical_array_find(MPI_PERS_ID, p_cont);
  struct mpi_pers_req_t* msg = NULL;
  int index;

  for(index = (array->nb_items)-1; index>=0; index--) {
    msg = ITH_ITEM(index, array);

    if(msg->mpi_req == mpi_req) {
      return msg;
    }
  }
  return NULL;
}

struct mpi_pers_req_t*
__pers_start(uint64_t __attribute__((unused)) time,
	     int rank,
	     app_ptr mpi_req)
{
  /* todo: record something ? (->number of messages sent with a pers_req ?) */
  struct mpi_request* req = __mpi_find_mpi_req(rank,
					       mpi_req,
					       mpi_req_none);

  return __mpi_find_pers_req(GET_PROCESS_CONTAINER(rank), req);
}
