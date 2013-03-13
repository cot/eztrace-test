#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <assert.h>
#include "mpi_ev_codes.h"
#include "eztrace_convert_mpi_p2p.h"
#include "eztrace_list.h"
#include "eztrace_array.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_hierarchical_array.h"

#if 0
/* Value of MPI_ANY_SOURCE */
extern int __MPI_ANY_SOURCE;
/* Value of MPI_ANY_TAG */
extern uint32_t __MPI_ANY_TAG;
#endif

enum pending_comm_status_t {
  NONE_DONE = 1,
  IRECV_DONE = 2,
  ISEND_DONE = 4,
  RWAIT_DONE = 8,
  SWAIT_DONE = 16,
};

struct mpi_pending_comm {
  struct ezt_list_token_t token;
  struct mpi_p2p_msg_t *msg;
  enum pending_comm_status_t status;
};


void init_mpi_p2p_messages() {
  hierarchical_array_attach(MPI_P2P_ID, sizeof(struct mpi_p2p_msg_t));
  hierarchical_array_attach(MPI_P2P_ISEND_ID, sizeof(struct p2p_msg_event));
  hierarchical_array_attach(MPI_P2P_IRECV_ID, sizeof(struct p2p_msg_event));
  hierarchical_array_attach(MPI_P2P_SWAIT_ID, sizeof(struct p2p_msg_event));
  hierarchical_array_attach(MPI_P2P_RWAIT_ID, sizeof(struct p2p_msg_event));

  hierarchical_array_attach(MPI_REQUEST_ID, sizeof(struct mpi_request));
}

void __print_mpi_req(struct mpi_request *req)
{
  printf("req %p(ptr %p, type %d): msg %p(src=%d, dest=%d, len=%d)\n",req, req->ptr, req->req_type,
	 req->msg, req->msg->src, req->msg->dest, req->msg->len);
}

void __print_mpi_requests_recurse(unsigned depth, p_eztrace_container p_cont)
{
  unsigned i;
  int index;
  struct hierarchical_array* array = hierarchical_array_find(MPI_REQUEST_ID, p_cont);
  assert(array);

  for(index = (array->nb_items)-1; index>=0; index--) {
    struct mpi_request *req = NULL;
    req = ITH_ITEM(index, array);
    assert(req);
    for(i=0; i<depth; i++)
      printf("  ");
    __print_mpi_req(req);
  }

  for(i=0; i<p_cont->nb_children; i++) {
    __print_mpi_requests_recurse(depth+1, p_cont->children[i]);
  }
}

void __print_mpi_requests()
{
  int i;
  for(i=0;i<NB_TRACES; i++) {
    p_eztrace_container p_cont = GET_PROCESS_CONTAINER(i);
    __print_mpi_requests_recurse(0, p_cont);
  }
}

void __print_p2p_message_header(FILE *stream)
{
  fprintf(stream, "# src\tdest\tlen\ttag");
  fprintf(stream, "\tIsend_tick\tSWait_tick\tWait_done_tick");
  fprintf(stream, "\tIrecv_tick\tRWait_tick\tWait_done_tick");
  fprintf(stream, "\tSender_request\tReceiver_request");
  fprintf(stream, "\n");
}

void __print_p2p_message(FILE *stream, struct mpi_p2p_msg_t *msg)
{
  fprintf(stream, "%d\t%d\t%d\t%x",
	  msg->src, msg->dest, msg->len, msg->tag);
  int i;
  for(i=0; i<P2P_NB_TIMES; i++)
    fprintf(stream, "\t%lld", msg->times[i]);

  fprintf(stream, "\t%p\t%p", msg->sender_request, msg->recver_request);
  fprintf(stream, "\n");
}

void __print_p2p_messages_recurse(FILE *stream, unsigned depth, p_eztrace_container p_cont)
{
  unsigned i;
  int index;
  struct hierarchical_array* array = hierarchical_array_find(MPI_P2P_ID, p_cont);
  assert(array);

  for(index = (array->nb_items)-1; index>=0; index--) {
    struct mpi_p2p_msg_t *msg = NULL;
    msg = ITH_ITEM(index, array);
    assert(msg);
    for(i=0;i<depth; i++)
      printf("  ");
    __print_p2p_message(stream, msg);
  }

  for(i=0; i<p_cont->nb_children; i++) {
    __print_p2p_messages_recurse(stream, depth+1, p_cont->children[i]);
  }
}

void __print_p2p_messages(FILE*stream)
{
  int i;
  __print_p2p_message_header(stream);
  for(i=0;i<NB_TRACES; i++) {
    /* reduce each trace counter */
    p_eztrace_container p_cont = GET_PROCESS_CONTAINER(i);

    __print_p2p_messages_recurse(stream, 0, p_cont);
  }
}

void __record_event(uint64_t time, p_eztrace_container p_cont,
		    int event_type, struct mpi_p2p_msg_t *p_msg)
{
  struct p2p_msg_event* msg = hierarchical_array_new_item(p_cont, event_type);
  msg->time = time;
  msg->msg = p_msg;
}

/* create a new p2p message and store it. */
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
					const struct mpi_request* recver_request)
{
  struct mpi_p2p_msg_t* msg = hierarchical_array_new_item(GET_PROCESS_CONTAINER(dest), MPI_P2P_ID);

  assert(msg);
  assert(dest >= 0);

  msg->id = id;
  msg->src = src;
  msg->dest = dest;
  msg->len = len;
  msg->tag = tag;
  msg->unexp = unexp;  /* set to 1 if the recv is detected before the send */
  int i;
  for(i=0; i<P2P_NB_TIMES; i++)
    msg->times[i] = 0;
  msg->link_value = link_value;
  msg->sender_thread_id = sender_thread_id;
  msg->recver_thread_id = recver_thread_id;

  msg->sender_request = sender_request;
  msg->recver_request = recver_request;

  return msg;
}

/* create a new mpi request and store it. */
struct mpi_request* __mpi_new_mpi_request(int rank,
					  app_ptr mpi_req,
					  enum mpi_request_type req_type)
{
  struct eztrace_container_t* container = GET_PROCESS_CONTAINER(rank_to_trace_id[rank]);
  struct mpi_request* req = hierarchical_array_new_item(container, MPI_REQUEST_ID);
  assert(req);

  req->container = container;
  req->ptr = mpi_req;
  req->req_type = req_type;
  req->msg = NULL;

  return req;
}

/* find an mpi request */
struct mpi_request*
__mpi_find_mpi_req(int rank,
		   app_ptr mpi_req,
		   enum mpi_request_type req_type)
{
  struct hierarchical_array *array = hierarchical_array_find(MPI_REQUEST_ID,
							     GET_PROCESS_CONTAINER(rank_to_trace_id[rank]));
  assert(array);

  int index;
  struct mpi_request* req = NULL;
  for(index = (array->nb_items)-1; index>=0; index--) {
    req = ITH_ITEM(index, array);

    if((req->ptr == mpi_req) &&
       ((req_type == mpi_req_none) ||
	(req->req_type == req_type))) {
      return req;
    }
  }
  return NULL;
}

/* find the p2p message that corresponds to an mpi request */
struct mpi_p2p_msg_t*
__mpi_find_p2p_message_by_mpi_req(int rank, const struct mpi_request* request)
{
  FUNC_NAME;
  assert(request);
  assert(request->req_type != mpi_req_none);

  if(request->msg)
    return request->msg;

  struct hierarchical_array* array = NULL;
  array = hierarchical_array_find(MPI_P2P_ID, request->container);
  assert(array);

  struct mpi_p2p_msg_t* msg = NULL;
  int index;

  /* fixme:
   * if request->msg is not set, we may be unable to find the message.
   * Since messages are stored in the receiver container, if the request is
   * an isend message, then we don't know who is the receiver.
   *
   * However, this should never happen (well, I hope so, we should check).
   */
  for(index = (array->nb_items)-1; index>=0; index--) {
    msg = ITH_ITEM(index, array);
    if(request->req_type == mpi_req_send) {
      if(request && (msg->src == rank) && (msg->sender_request == request)) {
	assert(msg->dest < NB_TRACES);
	return msg;
      }
    } else {
      if(request && (msg->dest == rank) && (msg->recver_request == request)) {
	assert(msg->src < NB_TRACES);
	return msg;
      }
    }
  }
  return NULL;
}

/* find a p2p message.
 * warning: this can be quite expensive since this function browse the whole message list!
 */
struct mpi_p2p_msg_t*
__mpi_find_p2p_message(int src __attribute__((unused)),
		       int dest,
		       int len __attribute__((unused)),
		       uint32_t tag,
		       int time_id)
{
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(dest), p_info);

  struct hierarchical_array* array = hierarchical_array_find(MPI_P2P_ID, GET_PROCESS_CONTAINER(dest));
  struct mpi_p2p_msg_t* msg = NULL;
  int index;

  /* browse the message list, starting from the end */
  for(index = (array->nb_items)-1; index>=0; index--) {
    msg = ITH_ITEM(index, array);

    if(((msg->src == src) || (msg->src == p_info->__MPI_ANY_SOURCE)) &&
       (msg->dest == dest) &&
       ((msg->tag == tag) || (msg->tag == p_info->__MPI_ANY_TAG)) &&
       (time_id<0 ||!msg->times[time_id])) {
      return msg;
    }
  }
  return NULL;
}

/* find a pending comm that matches(src, dest, len, tag) in the list
 * corresponding to type. Only consider pending comms that match the mask_ok
 * and doesn't match mask_not_ok
 */
struct mpi_pending_comm*
__mpi_p2p_find_pending_comm(int src,
			    int dest,
			    int len __attribute__((unused)),
			    uint32_t tag,
			    enum comm_type_t type,
			    uint32_t mask_ok,
			    uint32_t mask_not_ok)
{
  struct mpi_process_info_t * process_info = NULL;

  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(dest), p_info);

  switch (type) {
  case comm_type_outgoing:
    {
      INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(src), p_info_src);
      process_info=p_info_src;
    }
    break;
  case comm_type_incoming:
    {
      process_info=p_info;
    }
    break;
  default:
    assert(1);
  }


  struct mpi_p2p_msg_t* msg = NULL;
  struct ezt_list_token_t *token;

  /* for each pending comm in the dest process, check if the message matches */
  ezt_list_foreach(&process_info->pending_comm[type], token) {
    struct mpi_pending_comm* p_comm = (struct mpi_pending_comm*) token->data;
    assert(p_comm->msg);
    msg = p_comm->msg;

    if((msg->dest == dest) &&
       ((msg->src == src) || (msg->src == p_info->__MPI_ANY_SOURCE) || (src == p_info->__MPI_ANY_SOURCE)) &&
       ((msg->tag == tag) || (msg->tag == p_info->__MPI_ANY_TAG) || (tag == p_info->__MPI_ANY_TAG)) &&
       (p_comm->status & mask_ok) &&
       (!(p_comm->status & mask_not_ok))) {

      return p_comm;
    }
  }
  /* message not found */

  return NULL;
}

/* find a pending irecv */
struct mpi_pending_comm*
__mpi_p2p_find_pending_irecv(int src __attribute__((unused)),
			     int dest,
			     int len __attribute__((unused)),
			     uint32_t tag,
			     uint32_t mask_ok,
			     uint32_t mask_not_ok)
{
  return __mpi_p2p_find_pending_comm(src, dest, len, tag, comm_type_incoming, mask_ok, mask_not_ok);
}

/* find a pending isend */
struct mpi_pending_comm*
__mpi_p2p_find_pending_isend(int src,
			     int dest,
			     int len,
			     uint32_t tag,
			     uint32_t mask_ok,
			     uint32_t mask_not_ok)
{
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(dest), p_info);
  if(src == p_info->__MPI_ANY_SOURCE){
    /* In case of any_source, we don't know where to search for the isend,
     * so we need to find the message first
     */
    struct mpi_pending_comm* ret = NULL;
    int rank;
    int nb_traces = * get_nb_traces();

    /* search for a pending isend that match in all the processes */
    for(rank=0; rank<nb_traces; rank++) {
      ret = __mpi_p2p_find_pending_comm(rank, dest, len, tag, comm_type_outgoing, mask_ok, mask_not_ok);
      if(ret)
	goto out;
    }

  out:
    return ret;

  }
  return __mpi_p2p_find_pending_comm(src, dest, len, tag, comm_type_outgoing, mask_ok, mask_not_ok);
}

/* search for a message in a list of pending communication and remove it */
static void __remove_message_from_pending_comm(struct mpi_p2p_msg_t* msg,
					       int cont,
					       enum comm_type_t comm_type)
{
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(cont), process_info);
  struct ezt_list_token_t *token;
  ezt_list_foreach(&process_info->pending_comm[comm_type], token) {
    struct mpi_pending_comm* p_comm = (struct mpi_pending_comm*) token->data;

    if((p_comm->msg == msg) &&
       (p_comm->status == (NONE_DONE | IRECV_DONE | ISEND_DONE | RWAIT_DONE | SWAIT_DONE))) {
      ezt_list_remove(token);
      free(p_comm);
    }
  }
}

/* check for the completion of a message (ie. both send and recv ended)
 * and remove the message from the lists of pending communications
 */
static void __message_set_completed(struct mpi_p2p_msg_t* msg)
{
  int i;
  /* check wether the message is complete */
  for(i=0; i<P2P_NB_TIMES; i++) {
    if(!msg->times[i])
      return;
  }

  /* message is complete (ie. send and recv ended) */

  /* remove the message from the lists of pending sends */
  __remove_message_from_pending_comm(msg, msg->src, comm_type_outgoing);

  /* remove the message from the lists of pending receives */
  __remove_message_from_pending_comm(msg, msg->dest, comm_type_incoming);
}

static   struct mpi_pending_comm* __create_new_pending_comm(int process_id,
				      struct mpi_p2p_msg_t* msg,
				      enum comm_type_t type)
{
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(process_id), process_info);
  /* todo: use a block malloc instead (improved performance) */
  struct mpi_pending_comm* p_comm = malloc(sizeof(struct mpi_pending_comm));
  p_comm->status = NONE_DONE;
  p_comm->msg = msg;
  p_comm->token.data = p_comm;
  ezt_list_add(&process_info->pending_comm[type], &p_comm->token);

  return p_comm;
}

struct mpi_p2p_msg_t*
__start_recv_message(uint64_t start_time, int src, int dest, int len, uint32_t tag,
		     const char* thread_id, struct mpi_request* mpi_req)
{
  /* is there a corresponding isend ? */
  struct mpi_pending_comm* pending_comm = __mpi_p2p_find_pending_isend(src, dest, len, tag, NONE_DONE, IRECV_DONE);
  struct mpi_p2p_msg_t* msg = NULL;
  struct mpi_pending_comm* new_comm = NULL;
  if(!pending_comm) {
    /* no isend found. Create the message and register a irecv */
    msg = __new_p2p_message(NULL, src, dest, len, tag, 1, NULL, NULL, NULL, thread_id, mpi_req);
    pending_comm = __create_new_pending_comm(dest, msg, comm_type_incoming);
    pending_comm->status |= IRECV_DONE;

  } else {
    /* isend found, update its status */
    pending_comm->status |= IRECV_DONE;
    msg = pending_comm->msg;

    /* create a recv pending comm*/
    new_comm = __create_new_pending_comm(dest, msg, comm_type_incoming);
    new_comm->status |= pending_comm->status;

  }

  assert(msg);

  __record_event(start_time, GET_PROCESS_CONTAINER(dest), MPI_P2P_IRECV_ID, msg);
  msg->recver_thread_id = thread_id;
  msg->recver_request = mpi_req;
  msg->times[start_recv] = start_time;

  if(mpi_req)
    mpi_req->msg = msg;

  return msg;
}

struct mpi_p2p_msg_t*
__stop_recv_message(uint64_t stop_time,
		    int src,
		    int dest,
		    int len,
		    uint32_t tag,
		    const char* thread_id __attribute__((unused)),
		    struct mpi_request* mpi_req)
{
  /* retrieve the corresponding pending incoming comm */
  struct mpi_pending_comm* pending_comm = __mpi_p2p_find_pending_irecv(src, dest, len, tag, IRECV_DONE, RWAIT_DONE);
  struct mpi_p2p_msg_t* msg = NULL;
  struct mpi_pending_comm* pending_comm_outgoing = NULL;
  assert(pending_comm);


  if(!(pending_comm->status & ISEND_DONE))
    /* the matching isend has not happened yet. This means there's a synchronisation problem
     * between the traces.
     */
     return NULL;

  /* update the status of the pending incoming comm */
  pending_comm->status |= RWAIT_DONE;
  msg = pending_comm->msg;

  /* update the outgoing pending comm status */
  pending_comm_outgoing = __mpi_p2p_find_pending_isend(src, dest, len, tag, NONE_DONE, RWAIT_DONE);
  if(pending_comm_outgoing)
    pending_comm_outgoing->status |= RWAIT_DONE;

  assert(msg);

  __record_event(stop_time, GET_PROCESS_CONTAINER(dest), MPI_P2P_RWAIT_ID, msg);

  if(! msg->times[start_rwait])
    msg->times[start_rwait] = stop_time;
  msg->times[stop_recv] = stop_time;

  if(mpi_req)
    mpi_req->msg = NULL;

  if((!msg->id) &&
     msg->sender_thread_id &&
     msg->recver_thread_id) {
    CREATE_P2P_MSG_ID(msg);
  }

  if((!msg->link_value) &&
     msg->sender_thread_id &&
     msg->recver_thread_id) {
    CREATE_P2P_LINK_VALUE(msg);
  }

  __message_set_completed(msg);

  return msg;
}

struct mpi_p2p_msg_t*
__start_send_message(uint64_t start_time,
		     int src,
		     int dest,
		     int len,
		     uint32_t tag,
		     const char* thread_id,
		     struct mpi_request* mpi_req)
{
  /* did the matching irecv already happened ? */
  struct mpi_pending_comm* pending_comm = __mpi_p2p_find_pending_irecv(src, dest, len, tag, NONE_DONE, ISEND_DONE);
  struct mpi_p2p_msg_t* msg = NULL;
  struct mpi_pending_comm* new_comm = NULL;

  if(!pending_comm) {
    /* no irecv found. Create the message and register a isend */
    msg = __new_p2p_message(NULL, src, dest, len, tag, 0, NULL, thread_id, mpi_req, NULL, NULL);
    pending_comm = __create_new_pending_comm(src, msg, comm_type_outgoing);
    pending_comm->status |= ISEND_DONE;

  } else {
    /* irecv found, update its status */
    pending_comm->status |= ISEND_DONE;
    msg = pending_comm->msg;

    /* create a send pending comm*/
    new_comm = __create_new_pending_comm(src, msg, comm_type_outgoing);
    new_comm->status |= pending_comm->status;
  }

  assert(msg);

  __record_event(start_time, GET_PROCESS_CONTAINER(dest), MPI_P2P_ISEND_ID, msg);
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(dest), p_info);

  msg->sender_thread_id = thread_id;
  msg->sender_request = mpi_req;
  msg->times[start_send] = start_time;

  if(mpi_req)
    mpi_req->msg = msg;

 if(msg->src == p_info->__MPI_ANY_SOURCE)
    msg->src = src;
  if(msg->tag == p_info->__MPI_ANY_TAG)
    msg->tag = tag;

  if((!msg->id) &&
     msg->sender_thread_id &&
     msg->recver_thread_id) {
    CREATE_P2P_MSG_ID(msg);
  }

  if((!msg->link_value) &&
     msg->sender_thread_id &&
     msg->recver_thread_id) {
    CREATE_P2P_LINK_VALUE(msg);
  }

  return msg;
}

struct mpi_p2p_msg_t*
__stop_send_message(uint64_t stop_time,
		    int src,
		    int dest,
		    int len,
		    uint32_t tag,
		    const char* thread_id __attribute__((unused)),
		    struct mpi_request* mpi_req)
{
  /* retrieve the corresponding pending outgoing comm */
  struct mpi_pending_comm* pending_comm = __mpi_p2p_find_pending_isend(src, dest, len, tag, ISEND_DONE, SWAIT_DONE);
  struct mpi_p2p_msg_t* msg = NULL;
  assert(pending_comm);

  struct mpi_pending_comm* pending_comm_incoming = NULL;

  /* update the status of the outgoing comm*/
  pending_comm->status |= SWAIT_DONE;
  msg = pending_comm->msg;

  /* update the incoming pending comm status */
  pending_comm_incoming = __mpi_p2p_find_pending_irecv(src, dest, len, tag, NONE_DONE, SWAIT_DONE);
  if(pending_comm_incoming) {
    pending_comm_incoming->status |= SWAIT_DONE;
  }

  assert(msg);

  __record_event(stop_time, GET_PROCESS_CONTAINER(dest), MPI_P2P_SWAIT_ID, msg);
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(dest), p_info);

  if(msg->src == p_info->__MPI_ANY_SOURCE)
    msg->src = src;
  if(msg->tag == p_info->__MPI_ANY_TAG)
    msg->tag = tag;

  if(!msg->times[start_swait])
    msg->times[start_swait] = stop_time;
  msg->times[stop_send] = stop_time;

  if(mpi_req)
    mpi_req->msg = NULL;

  if((!msg->id) &&
     msg->sender_thread_id &&
     msg->recver_thread_id) {
    CREATE_P2P_MSG_ID(msg);
  }

  if((!msg->link_value) &&
     msg->sender_thread_id &&
     msg->recver_thread_id) {
    CREATE_P2P_LINK_VALUE(msg);
  }

  __message_set_completed(msg);

  return msg;
}
