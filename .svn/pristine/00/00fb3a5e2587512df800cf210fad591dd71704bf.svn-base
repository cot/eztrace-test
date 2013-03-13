/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_CONVERT_MPI_H
#define EZTRACE_CONVERT_MPI_H

#define _GNU_SOURCE
#include <stdio.h>
#include "eztrace_convert.h"

/* Different kinds of MPI messages */
enum MPI_MSG_TYPE {
  mpi_msg_type_p2p,
  mpi_msg_type_coll,
};

enum comm_type_t {
  comm_type_incoming= 0,   /* pending recv */
  comm_type_outgoing= 1,   /* pending send */
  comm_type_max  = 2,
};

/* Each process has a list of pending isend/irecv */
struct mpi_process_info_t {
  struct ezt_list_t pending_comm[comm_type_max];
  struct process_info_t *p_process;
  struct ezt_list_t communicators;
  int __MPI_ANY_SOURCE;  /* Value of MPI_ANY_SOURCE */
  uint32_t __MPI_ANY_TAG;  /* Value of MPI_ANY_TAG */
  app_ptr __MPI_REQUEST_NULL;  /* value of MPI_REQUEST_NULL */
  app_ptr __MPI_COMM_WORLD;	/* value of MPI_COMM_WORLD */
  app_ptr __MPI_COMM_SELF;	/* value of MPI_COMM_SELF */

};


struct ezt_mpi_comm {
  struct ezt_list_token_t token;
  app_ptr comm_id;
  int comm_size;
  int *ranks;
  /* useless ? */
  int my_rank; 		/* obtenu avec mpi_group_rank */
};

/* todo: remove this struct */
union mpi_msg {
  struct {
    enum {
      mpi_p2p_msg_send,
      mpi_p2p_msg_recv
    } p2p_type;
  } p2p;
  struct {
    enum{
      mpi_coll_msg_barrier,
      mpi_coll_msg_bcast
      /* todo: add other kinds of collective operations */
    } coll_type;
  } coll;
};


enum mpi_request_type{
  mpi_req_none,
  mpi_req_send,
  mpi_req_recv
};

struct mpi_request {
  app_ptr ptr;
  struct eztrace_container_t* container;
  enum mpi_request_type req_type;
  struct mpi_p2p_msg_t *msg;
};

enum p2p_time_ids{
  start_send = 0,
  start_swait = 1,
  stop_send = 2,
  start_recv = 3,
  start_rwait = 4,
  stop_recv = 5,
  P2P_NB_TIMES
};

struct mpi_p2p_msg_t {
  char*id;
  int src;
  int dest;
  int len;
  uint32_t tag;
  int unexp;  /* set to 1 if the recv is detected before the send */
  uint64_t times[P2P_NB_TIMES];
  char* link_value;
  const char* sender_thread_id;
  const struct mpi_request* sender_request;
  const char* recver_thread_id;
  const struct mpi_request* recver_request;
};

enum coll_type_t{
  mpi_coll_barrier,
  mpi_coll_bcast,
  mpi_coll_gather,
  mpi_coll_scatter,
  mpi_coll_allgather,
  mpi_coll_alltoall,
  mpi_coll_reduce,
  mpi_coll_allreduce,
  mpi_coll_reduce_scatter,
  mpi_coll_scan,
  /* todo: add other kinds of collective operations */
};

enum coll_time_ids{
  start_coll = 0,
  stop_coll = 2,
  COLL_NB_TIMES
};

struct mpi_coll_msg_t {
  enum coll_type_t type;
  uint64_t** times;
  int comm_size;
  int len;
  char***link_id;
  char*** link_value;
  int unexp;			/* todo: useless ? */
  char** thread_ids;
};


/* This structure contains information about a MPI message in 
   the global view (ie. it is valid no matter which process acces it)
*/
/* todo: cleanup this mess. some fields are only used for p2p messages !
 */
struct mpi_msg_t{
  char* id;
  enum MPI_MSG_TYPE msg_type;
  union mpi_msg msg;
  int src;
  int dest;
  int len;
  int unexp;  /* set to 1 if the recv is detected before the send */
  uint32_t tag;
  uint64_t send_time;
  uint64_t recv_time;
  uint64_t start_wait_time;
  char* link_value;
  char* threadstr;
};

/* This structure contains information about a MPI_COMM_SPAWN */
struct mpi_spawn_t{
  int nb_children;		/* number of processes spawned */
  uint64_t start_time;		/* date at which MPI_COMM_SPAWN was invoked */
  int ppid;			/* parent process id */
  struct trace_t *parent_trace;
};

/* This structure contains information on a persistent request */
struct mpi_pers_req_t{
  int process_id;
  enum mpi_request_type type;
  app_ptr buffer;
  int len;
  int src;
  int dest;
  uint32_t tag;
  struct mpi_request* mpi_req;
};

/* This structure contains informations on a MPI request.
 * it is only valid for one process (cf the rank field)
 */
struct __mpi_request_t {
  struct mpi_msg_t *msg;   /* pointer to the message corresponding to this request */
  union mpi_msg type;
  app_ptr req; /* address of the request in the MPI process */
  int rank;    /* the rank of the MPI process that uses this req */
};

/* This structure contains relevant informations used in the
 * communication matrix
 */
struct comm_info {
  int nb;
  int src;
  int dest;
  uint64_t size;
};

struct message_size {
  int nb;
  int size;
};

/* generate a MPI message id */
#define CREATE_P2P_MSG_ID(msg)						\
  {int __attribute__((unused)) ret = asprintf(&msg->id, "%d_%d_%20u_%p", msg->src, msg->dest, msg->tag, msg);}


/* generate the value of a p2p link */
#define CREATE_P2P_LINK_VALUE(msg)					\
  { int __attribute__((unused)) ret = asprintf(&(msg)->link_value, "src=%d, dest=%d, len=%d, tag=%x", \
					       (msg)->src, (msg)->dest, (msg)->len, (msg)->tag);}

/* generate the value of a coll link */
#define CREATE_COLL_MSG_ID(_msg_, _src_, _dest_)			\
  { int __attribute__((unused)) ret = asprintf(&(msg)->link_id[_src_][_dest_], "%d_%d_%p", _src_, _dest_, _msg_); }

#define CREATE_COLL_LINK_VALUE(_str_, _src_, _dest_)			\
  { int __attribute__((unused))ret = asprintf(&(_str_)->link_value[_src_][_dest_], "src=%d, dest=%d", \
					      _src_, _dest_); }

static inline struct comm_info *__create_comm_info(int src, int dest, int size, int nb){
  struct comm_info *msg = (struct comm_info*) malloc(sizeof(struct comm_info));
  msg->src = src;
  msg->dest = dest;
  msg->size = size;
  msg->nb = nb;
  return msg;
}

static inline struct message_size *__create_message_size(int size, int nb){
  struct message_size *msg = (struct message_size*) malloc(sizeof(struct message_size));
  msg->size = size;
  msg->nb = nb;
  return msg;
}

typedef enum {
 MPI_SEND_ID             ,
 MPI_RECV_ID             ,
 MPI_BSEND_ID            ,
 MPI_SSEND_ID            ,
 MPI_RSEND_ID            ,
 MPI_ISEND_ID            ,
 MPI_IBSEND_ID           ,
 MPI_ISSEND_ID           ,
 MPI_IRSEND_ID           ,
 MPI_IRECV_ID            ,
 MPI_SENDRECV_ID         ,
 MPI_SENDRECV_REPLACE_ID ,
 MPI_START_ID            ,
 MPI_STARTALL_ID         ,
 MPI_WAIT_ID             ,
 MPI_TEST_ID             ,
 MPI_WAITANY_ID          ,
 MPI_TESTANY_ID          ,
 MPI_WAITALL_ID          ,
 MPI_TESTALL_ID          ,
 MPI_WAITSOME_ID         ,
 MPI_TESTSOME_ID         ,
 MPI_PROBE_ID            ,
 MPI_IPROBE_ID           ,
 MPI_BARRIER_ID          ,
 MPI_BCAST_ID            ,
 MPI_GATHER_ID           ,
 MPI_GATHERV_ID          ,
 MPI_SCATTER_ID          ,
 MPI_SCATTERV_ID         ,
 MPI_ALLGATHER_ID        ,
 MPI_ALLGATHERV_ID       ,
 MPI_ALLTOALL_ID         ,
 MPI_ALLTOALLV_ID        ,
 MPI_REDUCE_ID           ,
 MPI_ALLREDUCE_ID        ,
 MPI_REDUCE_SCATTER_ID   ,
 MPI_SCAN_ID             ,
 MPI_GET_ID              ,
 MPI_PUT_ID              ,
 MPI_ID_SIZE
} MPI_id_t;

/* attach process-specific data to a process when it's created */
static struct mpi_process_info_t* __register_process_hook(struct process_info_t* p_process)
{
  struct mpi_process_info_t *p_info = (struct mpi_process_info_t*) malloc(sizeof(struct mpi_process_info_t));
  p_info->p_process = p_process;
  int i;
  for(i=0; i<comm_type_max; i++)
    ezt_list_new(&p_info->pending_comm[i]);

  p_info->__MPI_ANY_SOURCE = -1;  /* Value of MPI_ANY_SOURCE */
  p_info->__MPI_ANY_TAG = -1;  /* Value of MPI_ANY_TAG */
  p_info->__MPI_REQUEST_NULL = -1;  /* value of MPI_REQUEST_NULL */
  p_info->__MPI_COMM_WORLD = -1;	/* value of MPI_COMM_WORLD */
  p_info->__MPI_COMM_SELF = -1;	/* value of MPI_COMM_SELF */

  /* add the hook in the thread info structure */
  ezt_hook_list_add(&p_info->p_process->hooks, p_info, (uint8_t)MPI_EVENTS_ID);
  return p_info;
}

#define  INIT_MPI_PROCESS_INFO(p_process, var)				\
  struct mpi_process_info_t *var = (struct mpi_process_info_t*)		\
    ezt_hook_list_retrieve_data(&p_process->hooks, (uint8_t)MPI_EVENTS_ID); \
  if(!(var)) {								\
    var = __register_process_hook(p_process);				\
  }


static inline int ezt_get_global_rank(int trace_id, app_ptr comm, int local_rank)
{
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(trace_id), p_info);

  if(p_info->__MPI_COMM_WORLD == comm)
    return local_rank;

  if(p_info->__MPI_COMM_SELF == comm)
    return trace_id;

  /* the communicator in use isn't MPI_COMM_WORLD,
   * let's browse the list
   */
  struct ezt_list_token_t *token;
  ezt_list_foreach(&p_info->communicators, token) {
    struct ezt_mpi_comm* p_comm = (struct ezt_mpi_comm*) token->data;
    assert(p_comm);
    if(p_comm->comm_id == comm) {
      /* we found the communicator ! */
      assert(p_comm->comm_size > local_rank);
      return p_comm->ranks[local_rank];
    }
  }
  return -1;
}

extern int *rank_to_trace_id;
#define EZT_RANK_TO_TRACE_ID(rank) (rank_to_trace_id[rank])

#endif	/*  EZTRACE_CONVERT_MPI_H */
