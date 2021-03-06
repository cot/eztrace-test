/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <GTG.h>
#include <assert.h>
#include <limits.h>
#include "mpi_ev_codes.h"
#include "mpi_eztrace.h"
#include "eztrace_list.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_convert_mpi_p2p.h"
#include "eztrace_convert_mpi_coll.h"
#include "eztrace_convert_mpi_pers.h"
#include "eztrace_stats_mpi.h"

#ifdef USE_MPI
/* todo: we should get rid of these lists. Use the hierarchical arrays instead. */

/* list that contains pending MPI_Comm_spawn */
struct ezt_list_t spawn_list;

static int recording_stats = 0;
static unsigned nb_mpi_calls[MPI_ID_SIZE];

int *rank_to_trace_id = NULL;

#define MPI_CHANGE() if(!recording_stats) CHANGE()

static struct ezt_list_token_t* __find_matching_spawn(int ppid)
{
  struct ezt_list_token_t *t = NULL;
  struct mpi_spawn_t *r;
  ezt_list_foreach(&spawn_list, t){
    r = (struct mpi_spawn_t *) t->data;
    if(r->ppid == ppid)
      return t;
  }
  return t;
}

static struct mpi_p2p_msg_t* __mpi_send_generic(const char* thread_id, int src, int dest, int len, uint32_t tag,
			       struct mpi_request* mpi_req, app_ptr comm)
{

  int actual_dest = ezt_get_global_rank(rank_to_trace_id[src], comm, dest);
  assert(actual_dest!= -1);
  struct mpi_p2p_msg_t* msg = __start_send_message(CUR_TIME(CUR_INDEX), src, actual_dest, len, tag, thread_id, mpi_req);
  assert(msg);

  if(! msg->times[stop_recv]) {
#ifndef GTG_OUT_OF_ORDER
    MPI_CHANGE() startLink(CURRENT, "L_MPI_P2P", "C_Prog",
			   msg->sender_thread_id, msg->recver_thread_id,
			   msg->link_value,
			   msg->id);
#endif
  }
  return msg;
}


static void
__mpi_start_recv_generic (char*thread_id,
			  int src,
			  int dest,
			  int len,
			  int tag,
			  struct mpi_request* mpi_req,
			  app_ptr comm)
{

  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(rank_to_trace_id[dest]), p_info);
  int actual_src = src;
  if(src != p_info->__MPI_ANY_SOURCE) {
    actual_src = ezt_get_global_rank(rank_to_trace_id[dest], comm, src);
    assert(actual_src!= -1);
  }
  __start_recv_message(CUR_TIME(CUR_INDEX),
		       actual_src,
		       dest,
		       len,
		       tag,
		       thread_id, mpi_req);
}


/* return 1 if the matching isend didn't occured yet.
 * In that case, this function has to be called again when the matching isend occurs
 */
static int
__mpi_stop_recv_generic (char*thread_id,
			 int src,
			 int dest,
			 int len,
			 int tag,
			 struct mpi_request* mpi_req,
			 app_ptr comm)
{
  int delay_added = 0;
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(rank_to_trace_id[dest]), p_info);
  int actual_src = src;
  if(src != p_info->__MPI_ANY_SOURCE) {
    actual_src = ezt_get_global_rank(CUR_INDEX, comm, src);
    assert(actual_src!= -1);
  }


  struct mpi_p2p_msg_t *msg = __stop_recv_message(CUR_TIME(CUR_INDEX), actual_src, dest, len, tag, thread_id, mpi_req);
  if(!msg) {
    /* There is no matching MPI_Isend yet.
     * This means there's a synchronisation problem between the traces.
     * Let's wait until the remote process calls MPI_Isend
     */
    SKIP = 1;
    CUR_TRACE->skip = 1;
    return 1 ;
  }

  if(msg->times[start_send]) {

    if(msg->times[start_send] > msg->times[stop_recv]) {
      msg->times[stop_recv] += add_delay_to_trace(rank_to_trace_id[dest],
						  msg->times[stop_recv],
						  msg->times[start_send],
						  msg->recver_thread_id);

      delay_added = 1;
    }

    assert(msg->id);
    assert(msg->link_value);
    assert(msg->sender_thread_id);
    assert(msg->recver_thread_id);
#ifdef GTG_OUT_OF_ORDER
    MPI_CHANGE() startLink((float)(msg->times[start_send])/1000000.0,
			   "L_MPI_P2P", "C_Prog",
			   msg->sender_thread_id, msg->recver_thread_id,
			   msg->link_value, msg->id);

    MPI_CHANGE() endLink(msg->times[stop_recv]/1000000.0,
			 "L_MPI_P2P", "C_Prog",
			 msg->sender_thread_id, thread_id,
			 msg->link_value, msg->id);

    if(!delay_added) {

      char* message_str = NULL;
      float throughput = -1;
      /* duration in us */
      float duration = (CURRENT - ((float)(msg->times[start_send])/1000000.0))*1000;
      if(len) {
	/* byte / us = MB/s */
	throughput = len/duration;
      }
      asprintf(&message_str, "duration = %f us -- len=%d bytes -- throughput=%f MB/s",
	       duration, len, throughput);

      MPI_CHANGE() addEvent(CURRENT, "E_MPI_EndComm", thread_id, message_str);
    }
#else  /* GTG_OUT_OF_ORDER */
    MPI_CHANGE() endLink(CURRENT, "L_MPI_P2P", "C_Prog",
			 msg->sender_thread_id, msg->recver_thread_id,
			 msg->link_value, msg->id);
#endif
  }
  return 0;
}

void
handle_mpi_init ()
{
  int rank = GET_PARAM(CUR_EV, 1);

  if(!rank_to_trace_id) {
    int *nb = get_nb_traces();
    rank_to_trace_id = calloc(*nb, sizeof(int));
  }

  rank_to_trace_id[rank] = CUR_INDEX;

  CUR_TRACE->start_time = CUR_TRACE->ev.time;

  /* Check wether MPI_SPAWNED event has already occur.
   * In that case, the rank is already known.
   */
  if(!CUR_TRACE->start) {
    CUR_RANK = rank;
    CREATE_TRACE_ID_STR(CUR_ID, rank);
    GET_PROCESS_INFO(CUR_INDEX)->pid = rank;
    eztrace_create_ids(CUR_INDEX);

    CUR_TRACE->start = 1;
    NB_START++;

    /* Create the process and the main thread */
    DECLARE_PROCESS_ID_STR(process_id, CUR_INDEX);
    if(!recording_stats)addContainer (CURRENT, process_id, "CT_Process", "C_Prog", process_id, "0");
    new_thread(CUR_THREAD_ID);

  } else {
    CUR_TRACE->start = 1;
    NB_START++;
  }
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

  p_info->__MPI_ANY_SOURCE = GET_PARAM(CUR_EV, 3);
  p_info->__MPI_ANY_TAG = GET_PARAM(CUR_EV, 4);
  p_info->__MPI_REQUEST_NULL = (app_ptr) GET_PARAM(CUR_EV, 5);

  wait_for_an_event(CUR_TRACE->id, FUT_MPI_INIT_Info);

  p_info->__MPI_COMM_WORLD = (app_ptr) GET_PARAM(CUR_EV, 1);
  p_info->__MPI_COMM_SELF = (app_ptr) GET_PARAM(CUR_EV, 2);
  ezt_list_new(&p_info->communicators);

  FUNC_NAME;
}


void handle_mpi_spawned()
{
  /* set the start time */
  if(CUR_TRACE->start == 0)
    CUR_TRACE->start_time = CUR_EV->time;

  CUR_TRACE->start = 1;
  int ppid = GET_PARAM(CUR_EV, 1);

  struct ezt_list_token_t *token = __find_matching_spawn(ppid);
  if(!token) {
    /* the MPI_Comm_spawn has not been handled yet. We have to wait for it */
    SKIP = 1;
    CUR_TRACE->skip = 1;
    return;
  }

  struct mpi_spawn_t *spawn;
  spawn = (struct mpi_spawn_t *) token->data;
  /* We have to add a delay to all events on this trace */
  CUR_TRACE->delay = spawn->start_time;
  spawn->nb_children--;
  struct trace_t *parent_trace = spawn->parent_trace;

  if(!spawn->nb_children) {
    /* all the spawned processes have been handled, we can free this structure. */
    ezt_list_remove(token);
    free(spawn);
    free(token);
  }

  /* local_rank = rank amoung the spawned processes */
  int local_rank = GET_PARAM(CUR_EV, 2);
  CUR_RANK = ppid + local_rank;

  /* initialize the trace_id */
  asprintf(&CUR_ID, "%s_%d", parent_trace->trace_id, local_rank);
  eztrace_create_ids(CUR_INDEX);
  DECLARE_PROCESS_ID_STR(process_id, CUR_INDEX);
  if(!recording_stats)addContainer (CURRENT, process_id, "CT_Process", "C_Prog", process_id, "0");
  new_thread(CUR_THREAD_ID);

  /* Crappy hack: we know that next event is MPI_Init and
   * we need the trace to be properly started before continuing the function.
   */
  wait_for_an_event(CUR_TRACE->id, FUT_MPI_INIT);
  /* We should at least check if the code corresponds to MPI_INIT */
  handle_mpi_init ();

  FUNC_NAME;

  /* prepare the link value */
  char*link_id = NULL;
  int __attribute__((unused)) ret = asprintf(&link_id, "%s_%d", parent_trace->trace_id, local_rank);

  MPI_CHANGE() endLink (CURRENT,  "L_MPI_SPAWN", "C_Prog", parent_trace->trace_id, process_id, link_id, link_id);

  free(link_id);
}

void
handle_mpi_start_send ()
{
	if(_UNUSED_MPI<2)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int len = GET_PARAM(CUR_EV, 1);
		int dest = GET_PARAM(CUR_EV, 2);
		uint32_t tag = GET_PARAM(CUR_EV, 3);
		app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 4);

		struct mpi_p2p_msg_t* msg = __mpi_send_generic( thread_id, CUR_RANK, dest, len, tag, NULL, comm);

		if(!msg->times[start_swait])
			msg->times[start_swait] = CUR_TIME(CUR_INDEX);

		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Send");
	}
}


void
handle_mpi_stop_send ()
{
	if(_UNUSED_MPI<2)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int dest = GET_PARAM(CUR_EV, 1);
		int tag = GET_PARAM(CUR_EV, 2);
		app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 3);
		int actual_dest = ezt_get_global_rank(CUR_INDEX, comm, dest);
		assert(actual_dest!= -1);

		__stop_send_message(CUR_TIME(CUR_INDEX), CUR_RANK, actual_dest, -1, tag, thread_id, NULL);

		MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
	}
}

void
handle_mpi_start_recv ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  int len = GET_PARAM(CUR_EV, 1);
  int src = GET_PARAM(CUR_EV, 2);
  int dest = CUR_RANK;
  uint32_t tag = GET_PARAM(CUR_EV, 3);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 4);

  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);
  int actual_src = src;
  if(src != p_info->__MPI_ANY_SOURCE) {
    actual_src = ezt_get_global_rank(CUR_INDEX, comm, src);
    assert(actual_src!= -1);
  }

  struct mpi_p2p_msg_t* msg = __start_recv_message(CUR_TIME(CUR_INDEX),
						   actual_src, dest, len, tag, thread_id, NULL);

  if(!msg->times[start_rwait])
    msg->times[start_rwait] = CUR_TIME(CUR_INDEX);

  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Recv");
}

void
handle_mpi_stop_recv ()
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int len = GET_PARAM(CUR_EV, 1);
  int src = GET_PARAM(CUR_EV, 2);
  uint32_t tag = GET_PARAM(CUR_EV, 3);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 4);
  if(__mpi_stop_recv_generic (thread_id, src, CUR_RANK, len, tag, NULL, comm)) {
    return;
  }

  MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}

void handle_mpi_start_sendrecv ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int len = GET_PARAM(CUR_EV, 1);
  int dest = GET_PARAM(CUR_EV, 2);
  int src = CUR_RANK;
  uint32_t tag = GET_PARAM(CUR_EV, 3);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 4);

  /* process the send */
  struct mpi_p2p_msg_t* msg = __mpi_send_generic( thread_id, src, dest, len, tag, NULL, comm);

  if(!msg->times[start_swait])
    msg->times[start_swait] = CUR_TIME(CUR_INDEX);

  /* process the recv */
  wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);

  len = GET_PARAM(CUR_EV, 1);
  src = GET_PARAM(CUR_EV, 2);
  dest = CUR_RANK;
  tag = GET_PARAM(CUR_EV, 3);

  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);
  int actual_src = src;
  if(src != p_info->__MPI_ANY_SOURCE) {
    actual_src = ezt_get_global_rank(CUR_INDEX, comm, src);
    assert(actual_src!= -1);
  }

  msg = __start_recv_message(CUR_TIME(CUR_INDEX),
			     actual_src, dest, len, tag, thread_id, NULL);

  if(!msg->times[start_rwait])
    msg->times[start_rwait] = CUR_TIME(CUR_INDEX);


  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Sendrecv");
}

void handle_mpi_stop_sendrecv ()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  /* we need to process the recv first because we may have to re-synchronize traces
   * and thus, we may have to call handle_mpi_sop_sendrecv several times (until the corresponding
   * send occurs).
   */
  int len = GET_PARAM(CUR_EV, 1);
  int src = GET_PARAM(CUR_EV, 2);
  int dest = CUR_RANK;
  uint32_t tag = GET_PARAM(CUR_EV, 3);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 4);

  /* process the recv */
  if(__mpi_stop_recv_generic (thread_id, src, CUR_RANK, len, tag, NULL, comm)) {
    return;
  }

  /* process the send */
  wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);

  len = GET_PARAM(CUR_EV, 1);
  src = CUR_RANK;
  dest = GET_PARAM(CUR_EV, 2);
  tag = GET_PARAM(CUR_EV, 3);

  int actual_dest = ezt_get_global_rank(CUR_INDEX, comm, dest);
  __stop_send_message(CUR_TIME(CUR_INDEX), CUR_RANK, actual_dest, -1, tag, thread_id, NULL);

  /* pop the sendrecv state */
  MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}

void handle_mpi_start_sendrecv_replace ()
{
  handle_mpi_start_sendrecv();
  /* todo: implement this */
}

void handle_mpi_stop_sendrecv_replace ()
{
  handle_mpi_stop_sendrecv();
}

/* todo: implement this ! */
void handle_mpi_start_bsend()
{
  /* for now, let's assume that bsend and send are the same */
  handle_mpi_start_send ();
}

void handle_mpi_stop_bsend()
{
  handle_mpi_stop_send ();
}

void handle_mpi_start_ssend()
{
  handle_mpi_start_send ();
}

void handle_mpi_stop_ssend()
{
  handle_mpi_stop_send ();
}

void handle_mpi_start_rsend()
{
  handle_mpi_start_send ();
}

void handle_mpi_stop_rsend()
{
  handle_mpi_stop_send ();
}

void handle_mpi_isend()
{
  FUNC_NAME;

  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int len = GET_PARAM(CUR_EV, 1);
  __attribute__((unused)) int src = CUR_RANK;
  int dest = GET_PARAM(CUR_EV, 2);
  uint32_t tag = GET_PARAM(CUR_EV, 3);
  app_ptr app_req = (app_ptr)GET_PARAM(CUR_EV, 4);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 5);

  struct mpi_request * mpi_req =  __mpi_new_mpi_request(CUR_RANK,
							app_req,
							mpi_req_send);
  assert(mpi_req);

  __mpi_send_generic( thread_id, CUR_RANK, dest, len, tag, mpi_req, comm);

  /* todo: add a send event here */
  MPI_CHANGE() pushState (CURRENT, "ST_Thread", thread_id, "STV_MPI_Overlap");
}

void handle_mpi_ibsend()
{
  handle_mpi_isend();
}

void handle_mpi_issend()
{
  handle_mpi_isend();
}

void handle_mpi_irsend()
{
  handle_mpi_isend();
}

void handle_mpi_irecv()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int len = GET_PARAM(CUR_EV, 1);
  int src = GET_PARAM(CUR_EV, 2);
  int dest = CUR_RANK;
  uint32_t tag = GET_PARAM(CUR_EV, 3);
  app_ptr app_req = (app_ptr) GET_PARAM(CUR_EV, 4);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 5);
  struct mpi_request * mpi_req =  __mpi_new_mpi_request(CUR_RANK, app_req, mpi_req_recv);
  assert(mpi_req);
  __mpi_start_recv_generic (thread_id, src, dest, len, tag, mpi_req, comm);
  MPI_CHANGE() pushState (CURRENT, "ST_Thread", thread_id, "STV_MPI_Overlap");
}

void handle_mpi_start_put()
{
  handle_mpi_start_send ();
}

void handle_mpi_stop_put()
{
  handle_mpi_stop_send ();
}

void handle_mpi_start_get()
{
  handle_mpi_start_recv ();
}

void handle_mpi_stop_get()
{
  handle_mpi_stop_recv ();
}

void handle_mpi_start_wait()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);

  app_ptr mpi_req = (app_ptr) GET_PARAM(CUR_EV, 1);
  struct mpi_request * req = __mpi_find_mpi_req(CUR_RANK, mpi_req, mpi_req_none);
  assert(req);
  struct mpi_p2p_msg_t* msg = __mpi_find_p2p_message_by_mpi_req(CUR_RANK, req);
  assert(msg);

  if(req->req_type == mpi_req_recv) {
    /*  this is a receive request */
    msg->times[start_rwait] = CUR_TIME(CUR_INDEX);
    MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Recv");
  } else {
    /*  this is a send request */
    msg->times[start_swait] = CUR_TIME(CUR_INDEX);
    MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Send");
  }
}


void handle_mpi_start_waitany()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
		INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

		MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);

		int nb_reqs = GET_PARAM(CUR_EV, 1);
		int i;
		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Waitany");

		for(i=0; i<nb_reqs; i++) {
			wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);
			app_ptr mpi_req = (app_ptr) GET_PARAM(CUR_EV, 1);
			if(mpi_req == p_info->__MPI_REQUEST_NULL)
				continue;

			struct mpi_request * req = __mpi_find_mpi_req(CUR_RANK, mpi_req, mpi_req_none);
			assert(req);
			struct mpi_p2p_msg_t* msg = __mpi_find_p2p_message_by_mpi_req(CUR_RANK, req);
			assert(msg);

			if(req->req_type == mpi_req_recv) {
				/*  this is a receive request */
				msg->times[start_rwait] = CUR_TIME(CUR_INDEX);
			} else {
				/*  this is a send request */
				msg->times[start_swait] = CUR_TIME(CUR_INDEX);
			}
		}
	}
}

void handle_mpi_start_waitall()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
		INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

		int nb_reqs = GET_PARAM(CUR_EV, 1);
		int i;
		double cur_time = CURRENT;

		for(i=0; i<nb_reqs; i++) {
			wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);
			app_ptr mpi_req = (app_ptr) GET_PARAM(CUR_EV, 1);
			if(mpi_req == p_info->__MPI_REQUEST_NULL)
				continue;

			MPI_CHANGE() popState(cur_time, "ST_Thread", thread_id);

			struct mpi_request * req = __mpi_find_mpi_req(CUR_RANK, mpi_req, mpi_req_none);
			if(!req) {
				/* this means the request was already freed, let's skip this one */
				continue;
			}
			struct mpi_p2p_msg_t* msg = __mpi_find_p2p_message_by_mpi_req(CUR_RANK, req);
			assert(msg);

			if(req->req_type == mpi_req_recv) {
				/*  this is a receive request */
				msg->times[start_rwait] = CUR_TIME(CUR_INDEX);
			} else {
				/*  this is a send request */
				msg->times[start_swait] = CUR_TIME(CUR_INDEX);
			}
		}

		MPI_CHANGE() pushState(cur_time, "ST_Thread", thread_id, "STV_MPI_Waitall");
	}
}

static int __handle_mpi_test_success(app_ptr req)
{
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

  struct mpi_request * mpi_req = __mpi_find_mpi_req(CUR_RANK, req, mpi_req_none);
  if(!mpi_req) {
    /* this means the request was already freed, let's skip this one */
    return 1;
  }
  struct mpi_p2p_msg_t* msg = __mpi_find_p2p_message_by_mpi_req(CUR_RANK, mpi_req);
  assert(msg);

  if(mpi_req->req_type == mpi_req_recv) {
    /* this is a recv request */
    assert(msg->recver_request == mpi_req);
    if(__mpi_stop_recv_generic(thread_id, msg->src, msg->dest, msg->len, msg->tag, mpi_req,p_info->__MPI_COMM_WORLD))
      return 0;
  } else {
    assert(mpi_req->req_type == mpi_req_send);
    assert(msg->sender_request == mpi_req);

    /* this is a send request */
    __stop_send_message(CUR_TIME(CUR_INDEX), msg->src, msg->dest, msg->len, msg->tag, thread_id, mpi_req);
  }
  return 1;
}

static void handle_mpi_start_probe()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Probe");
}

static void handle_mpi_stop_probe()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  char* probe_str = NULL;
  int src = GET_PARAM(CUR_EV, 1);
  uint32_t tag = GET_PARAM(CUR_EV, 2);
  int len = GET_PARAM(CUR_EV, 3);
  asprintf(&probe_str, "src=%d, tag=%x, length=%d", src, tag, len);

  MPI_CHANGE() addEvent(CURRENT, "E_MPI_Probe_success", thread_id, probe_str);
  MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);

  free(probe_str);
}

void handle_mpi_iprobe_success()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  char* probe_str = NULL;
  int src = GET_PARAM(CUR_EV, 1);
  uint32_t tag = GET_PARAM(CUR_EV, 2);
  int len = GET_PARAM(CUR_EV, 3);
  asprintf(&probe_str, "src=%d, tag=%x, length=%d", src, tag, len);
  MPI_CHANGE() addEvent(CURRENT, "E_MPI_Iprobe_success", thread_id, probe_str);

  free(probe_str);
}

void handle_mpi_iprobe_failed()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  char* probe_str = NULL;
  int src = GET_PARAM(CUR_EV, 1);
  uint32_t tag = GET_PARAM(CUR_EV, 2);
  asprintf(&probe_str, "src=%d, tag=%x", src, tag);
  MPI_CHANGE() addEvent(CURRENT, "E_MPI_Iprobe_failed", thread_id, probe_str);

  free(probe_str);
}

void handle_mpi_stop_wait()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  app_ptr req = (app_ptr) GET_PARAM(CUR_EV, 1);
  if(!__handle_mpi_test_success(req))
    return;

  MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}

void handle_mpi_stop_waitany()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
		INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

		int nb_reqs = GET_PARAM(CUR_EV, 1);
		int index = GET_PARAM(CUR_EV, 2);

		/* process each request*/
		int i;
		for(i=0; i<nb_reqs ; i++) {
			wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);
			app_ptr mpi_req = (app_ptr) GET_PARAM(CUR_EV, 1);

			if(index == i) {
				/* the successful request */
				if(__handle_mpi_test_success(mpi_req))
					return;
			} else {
				if(mpi_req == p_info->__MPI_REQUEST_NULL)
					continue;
				/* the other requests */
				struct mpi_request * req = __mpi_find_mpi_req(CUR_RANK, mpi_req, mpi_req_none);
				assert(req);
				struct mpi_p2p_msg_t* msg = __mpi_find_p2p_message_by_mpi_req(CUR_RANK, req);
				assert(msg);

				if(req->req_type == mpi_req_recv) {
					/*  this is a receive request */
					msg->times[start_rwait] = 0;
				} else {
					/*  this is a send request */
					msg->times[start_swait] = CUR_TIME(CUR_INDEX);
				}
			}
		}
		MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
	}
}

struct mpi_stop_waitall_replay{
  int nb_reqs;
  int i;
  app_ptr mpi_req;
};

void handle_mpi_stop_waitall( struct mpi_stop_waitall_replay *r)
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
		INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

		int nb_reqs;
		if(r)
			nb_reqs = r->nb_reqs;
		else
			nb_reqs = GET_PARAM(CUR_EV, 1);

		/* process each request*/
		int i = 0;
		if(r)
			i = r->i;
		for(; i<nb_reqs ; i++) {

			if(!(r && r->i == i)){
				/* unless this is the replay first look, fetch next event */
				wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);
			}

			app_ptr mpi_req = (app_ptr) GET_PARAM(CUR_EV, 1);

			if(r && (r->i == i))
				assert(mpi_req == r->mpi_req);

			if(mpi_req == p_info->__MPI_REQUEST_NULL)
				continue;

			if(! __handle_mpi_test_success(mpi_req)){
				/* The matching isend has not occured yet. We need to wait until it happens */

				struct mpi_stop_waitall_replay* replay = malloc(sizeof(struct mpi_stop_waitall_replay));
				replay->nb_reqs = nb_reqs;
				replay->i = i;
				replay->mpi_req = mpi_req;

				ask_for_replay(CUR_INDEX, (void (*)(void *)) handle_mpi_stop_waitall, replay);

				goto out;
			}
		}
		MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
out:
		if( r )
			free(r);
		return;
	}
}

void handle_mpi_test_success()
{
  FUNC_NAME;
  app_ptr mpi_req = (app_ptr) GET_PARAM(CUR_EV, 1);
  if(__handle_mpi_test_success(mpi_req))
    return ;
}

#define start_coll_link(msg, src, dest)\
  startLink (msg->times[src][start_coll] / 1000000.0,			\
	     "L_MPI_Coll", "C_Prog",					\
	     msg->thread_ids[src], msg->thread_ids[dest],		\
	     msg->link_value[src][dest], msg->link_id[src][dest])

#define end_coll_link(msg, src, dest)					\
  endLink(msg->times[dest][stop_coll] / 1000000.0,			\
    "L_MPI_Coll", "C_Prog",						\
	  msg->thread_ids[src], msg->thread_ids[dest],			\
	  msg->link_value[src][dest], msg->link_id[src][dest])


static void __mpi_barrier_start_generic(struct mpi_coll_msg_t* msg, int my_rank)
{
  int i;
  for(i=0; i<msg->comm_size; i++) {
    if(i != my_rank) {

#ifdef GTG_OUT_OF_ORDER
      if(msg->times[i][start_coll]) {
	/* the remote process start_coll already happened.
	 * This means that we know both sender and recver threads, so
	 * we can create the links
	 */
	/* start_link(my_rank, i) */
	assert(msg->thread_ids[my_rank]);
	assert(msg->thread_ids[i]);

	MPI_CHANGE() start_coll_link(msg, my_rank, i);
	MPI_CHANGE() start_coll_link(msg, i, my_rank);
      }

      if(msg->times[i][stop_coll]) {
	/* the remote process stop_coll already happened.
	 * This means that there's a synchronisation problem between the traces
	 */

	msg->times[i][stop_coll] += add_delay_to_trace(i,
						       msg->times[i][stop_coll],
						       msg->times[my_rank][start_coll],
						       msg->thread_ids[i]);
      }
#else
      /* todo: not implemented yet */
#endif	/* GTG_OUT_OF_ORDER */
    }
  }
}


static void __mpi_barrier_stop_generic(struct mpi_coll_msg_t* msg, int my_rank)
{
  int i;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  uint64_t my_stop_time = msg->times[my_rank][stop_coll];

  for(i=0; i<msg->comm_size; i++) {
    if(i != my_rank) {
      if(! msg->times[i][start_coll]) {
	/* ith process has not yet reached the MPI_Barrier.
	 * This means there's a synchronisation problem between the traces.
	 * Let's wait until the remote process starts MPI_Barrier
	 */
	SKIP = 1;
	CUR_TRACE->skip = 1;
	msg->times[my_rank][stop_coll] = 0;
	return ;
      }

      if(msg->times[i][start_coll] > my_stop_time) {
	my_stop_time = msg->times[i][start_coll];
      }
    }
  }

  if(my_stop_time > msg->times[my_rank][stop_coll]) {
    msg->times[my_rank][stop_coll] += add_delay_to_trace(my_rank,
							 msg->times[my_rank][stop_coll],
							 my_stop_time,
							 msg->thread_ids[my_rank]);

  }


  for(i=0; i<msg->comm_size; i++) {
    if(i != my_rank) {

      if(msg->times[i][stop_coll]) {
#ifdef GTG_OUT_OF_ORDER
	/* the remote process stop_coll already happened.
	 * This means that we know both sender and recver endlink time, so
	 * we can end both links
	 */
	/* endlink(my_rank, i) */
	assert(msg->thread_ids[my_rank]);
	assert(msg->thread_ids[i]);
	MPI_CHANGE() end_coll_link(msg, my_rank, i);
	MPI_CHANGE() end_coll_link(msg, i, my_rank);

#else
	/* todo: not implemented yet */
#endif /* GTG_OUT_OF_ORDER */
      }
    }
  }
  MPI_CHANGE() popState(CURRENT, "ST_Thread", thread_id);
}

void handle_mpi_start_BCast()
{
	if(_UNUSED_MPI==0)
	{
		/* some collective communications won't work with communicators != COMM_WORLD
		 * (rank in the wrong communicator used)
		 */
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_BCast");
		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);
		//int comm = GET_PARAM(CUR_EV, 1);
		int len = GET_PARAM(CUR_EV, 4);
		int __attribute__((unused)) root = GET_PARAM(CUR_EV, 5);

#if 0
		/* todo: implement this with hierarchical arrays */
		if (my_rank != root){
			__register_comm_info(root, my_rank, len);
		}
#endif

		struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_bcast,
				comm_size, my_rank,
				len, thread_id);
		__mpi_barrier_start_generic(msg, my_rank);
	}
}

void handle_mpi_start_Gather()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Gather");
  int my_rank = GET_PARAM(CUR_EV, 3);
  int comm_size = GET_PARAM(CUR_EV, 2);
  //int comm = GET_PARAM(CUR_EV, 1);
  int len = GET_PARAM(CUR_EV, 4);
  int __attribute__((unused)) root = GET_PARAM(CUR_EV, 5);

#if 0
  /* todo: implement this with hierarchical arrays */
  if (my_rank != root){
    __register_comm_info(my_rank, root, len);
  }
#endif

  struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_gather,
					    comm_size, my_rank,
					    len, thread_id);

  __mpi_barrier_start_generic(msg, my_rank);

}

void handle_mpi_start_Gatherv()
{
  handle_mpi_start_Gather();
}

void handle_mpi_start_Scatter()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Scatter");
  int my_rank = GET_PARAM(CUR_EV, 3);
  int comm_size = GET_PARAM(CUR_EV, 2);
  //int comm = GET_PARAM(CUR_EV, 1);
  int len = GET_PARAM(CUR_EV, 4);
  int __attribute__((unused)) root = GET_PARAM(CUR_EV, 5);

#if 0
  /* todo: implement this with hierarchical arrays */
  if (my_rank != root){
    __register_comm_info(root, my_rank, len);
  }
#endif
  struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_scatter,
					    comm_size, my_rank,
					    len, thread_id);

  __mpi_barrier_start_generic(msg, my_rank);
}

void handle_mpi_start_Scatterv()
{
  handle_mpi_start_Scatter();
}

void handle_mpi_start_Allgather()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Allgather");
		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);
		//int comm = GET_PARAM(CUR_EV, 1);
		int len = GET_PARAM(CUR_EV, 4);

#if 0
		int i;

		/* todo: implement this with hierarchical arrays */
		for(i=0; i<comm_size; i++){
			if(i != my_rank){
				__register_comm_info(my_rank, i, len);
			}
		}
#endif

		struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_allgather,
				comm_size, my_rank,
				len, thread_id);

		__mpi_barrier_start_generic(msg, my_rank);

	}

}

void handle_mpi_start_Allgatherv()
{
  handle_mpi_start_Allgather();
}

void handle_mpi_start_Alltoall()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Alltoall");
		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);
		//int comm = GET_PARAM(CUR_EV, 1);

		struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_alltoall,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */
		__mpi_barrier_start_generic(msg, my_rank);
	}
}

void handle_mpi_start_Alltoallv()
{
  handle_mpi_start_Alltoall();
}

void handle_mpi_start_Reduce()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Reduce");
		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);

		struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_reduce,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */
		__mpi_barrier_start_generic(msg, my_rank);
	}
}

void handle_mpi_start_Allreduce()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Allreduce");
		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);

		struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_allreduce,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */
		__mpi_barrier_start_generic(msg, my_rank);
	}
}

void handle_mpi_start_Reduce_scatter()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_reduce_scatter");
		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);

		struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_reduce_scatter,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */
		__mpi_barrier_start_generic(msg, my_rank);
	}
}

void handle_mpi_start_Scan()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Scan");
  int my_rank = GET_PARAM(CUR_EV, 3);
  int comm_size = GET_PARAM(CUR_EV, 2);

  struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_scan,
					    comm_size, my_rank,
					    0, thread_id); /* todo: add the len */
  __mpi_barrier_start_generic(msg, my_rank);
}

void handle_mpi_start_barrier()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Barrier");

  int my_rank = GET_PARAM(CUR_EV, 2);
  int comm_size = GET_PARAM(CUR_EV, 3);

  struct mpi_coll_msg_t* msg = __enter_coll(CUR_TIME(CUR_INDEX), mpi_coll_barrier,
					    comm_size, my_rank,
					    0, thread_id);

  __mpi_barrier_start_generic(msg, my_rank);
}


void handle_mpi_stop_BCast()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);
		int len = GET_PARAM(CUR_EV, 4);

#if 1
		struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_bcast,
				comm_size, my_rank,
				len, thread_id);
#else
		struct mpi_coll_msg_t* msg = __mpi_find_coll_message(mpi_coll_bcast, comm_size, my_rank, len, stop_coll);
#endif

		__mpi_barrier_stop_generic(msg, my_rank);
	}
}

void handle_mpi_stop_Gather()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int my_rank = GET_PARAM(CUR_EV, 3);
  int comm_size = GET_PARAM(CUR_EV, 2);
  int len = GET_PARAM(CUR_EV, 4);

  struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_gather,
					    comm_size, my_rank,
					    len, thread_id);

  __mpi_barrier_stop_generic(msg, my_rank);
}

void handle_mpi_stop_Gatherv()
{
  handle_mpi_stop_Gather();
}

void handle_mpi_stop_Scatter()
{
  //  handle_mpi_stop_BCast();
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int my_rank = GET_PARAM(CUR_EV, 3);
  int comm_size = GET_PARAM(CUR_EV, 2);
  int len = GET_PARAM(CUR_EV, 4);

  struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_scatter,
					    comm_size, my_rank,
					    len, thread_id);

  __mpi_barrier_stop_generic(msg, my_rank);
}

void handle_mpi_stop_Scatterv()
{
  handle_mpi_stop_Scatter();
}

void handle_mpi_stop_Allgather()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);
		int len = GET_PARAM(CUR_EV, 4);

		struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_allgather,
				comm_size, my_rank,
				len, thread_id);

		__mpi_barrier_stop_generic(msg, my_rank);
	}
}

void handle_mpi_stop_Allgatherv()
{
  handle_mpi_stop_Allgather();
}

void handle_mpi_stop_Alltoall()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);

		struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_alltoall,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */

		__mpi_barrier_stop_generic(msg, my_rank);
	}
}

void handle_mpi_stop_Alltoallv()
{
  handle_mpi_stop_Alltoall();
}

void handle_mpi_stop_Reduce()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);

		struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_reduce,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */

		__mpi_barrier_stop_generic(msg, my_rank);
	}
}

void handle_mpi_stop_Allreduce()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);

		struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_allreduce,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */

		__mpi_barrier_stop_generic(msg, my_rank);
	}
}

void handle_mpi_stop_Reduce_scatter()
{
	if(_UNUSED_MPI==0)
	{
		FUNC_NAME;
		DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

		int my_rank = GET_PARAM(CUR_EV, 3);
		int comm_size = GET_PARAM(CUR_EV, 2);

		struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_reduce_scatter,
				comm_size, my_rank,
				0, thread_id); /* todo: add the len */

		__mpi_barrier_stop_generic(msg, my_rank);
	}
}

void handle_mpi_stop_Scan()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int my_rank = GET_PARAM(CUR_EV, 3);
  int comm_size = GET_PARAM(CUR_EV, 2);

  struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_scan,
					    comm_size, my_rank,
					    0, thread_id); /* todo: add the len */

  __mpi_barrier_stop_generic(msg, my_rank);
}

void handle_mpi_stop_barrier()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);

  int my_rank = GET_PARAM(CUR_EV, 2);;
  int comm_size = GET_PARAM(CUR_EV, 3);

  struct mpi_coll_msg_t* msg = __leave_coll(CUR_TIME(CUR_INDEX), mpi_coll_barrier,
					    comm_size, my_rank,
					    0, thread_id);

  __mpi_barrier_stop_generic(msg, my_rank);
}

void handle_mpi_spawn()
{
  FUNC_NAME;
  /* This event happens when a process calls MPI_Comm_spawn
   * We have to:
   * - add the process info (PID, etc.) to the spawn_list
   * - create a few links
   */
  int pid = GET_PARAM(CUR_EV, 1);
  int nb_spawned = GET_PARAM(CUR_EV, 2);

  char *link_prefix = NULL;
  int ret = asprintf(&link_prefix, "%s_", CUR_ID);
  assert(ret>=0);
  int i;

  /* add the current spawn to the list of pending spawns */
  struct mpi_spawn_t *spawn = malloc(sizeof(struct mpi_spawn_t));
  spawn->nb_children = nb_spawned;
  spawn->start_time = CUR_TIME(CUR_INDEX);
  spawn->ppid = pid;
  spawn->parent_trace = CUR_TRACE;

  struct ezt_list_token_t* token = malloc(sizeof(struct ezt_list_token_t));
  token->data = spawn;
  ezt_list_add(&spawn_list, token);

  /* start one link per spawned process */
  for(i=0; i<nb_spawned; i++) {
    char *link_id = NULL;
    ret = asprintf(&link_id, "%s%d", link_prefix, i);
    assert(ret>=0);

    MPI_CHANGE() startLink (CURRENT, "L_MPI_SPAWN", "C_Prog", CUR_ID, NULL,
                        link_id, link_id);
    free(link_id);
  }

  free(link_prefix);
}


void handle_mpi_send_init()
{
  FUNC_NAME;

  app_ptr buffer = (app_ptr)GET_PARAM(CUR_EV, 1);
  int len = GET_PARAM(CUR_EV, 2);
  int dest = GET_PARAM(CUR_EV, 3);
  int src = CUR_RANK;

  wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);

  uint32_t tag = GET_PARAM(CUR_EV, 1);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 2);
  app_ptr mpi_req = (app_ptr)GET_PARAM(CUR_EV, 3);

  int actual_dest = ezt_get_global_rank(CUR_INDEX, comm, dest);
  assert(actual_dest!= -1);

  struct mpi_pers_req_t __attribute__((unused)) *req = __pers_init(CURRENT, mpi_req_send, buffer,
								   src, actual_dest, len, tag, mpi_req);

  DECLARE_CUR_THREAD(cur_thread);
}

void handle_mpi_bsend_init(){
  handle_mpi_send_init();
}

void handle_mpi_rsend_init()
{
  handle_mpi_send_init();
}

void handle_mpi_ssend_init()
{
  handle_mpi_send_init();
}

void handle_mpi_recv_init()
{
  FUNC_NAME;
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

  app_ptr buffer = (app_ptr)GET_PARAM(CUR_EV, 1);
  int len = GET_PARAM(CUR_EV, 2);
  int src = GET_PARAM(CUR_EV, 3);
  int dest = CUR_RANK;

  wait_for_an_event(CUR_TRACE->id, FUT_MPI_Info);

  uint32_t tag = GET_PARAM(CUR_EV, 1);
  app_ptr comm = (app_ptr)GET_PARAM(CUR_EV, 2);
  app_ptr mpi_req = (app_ptr)GET_PARAM(CUR_EV, 3);

  int actual_src = src;
  if(src != p_info->__MPI_ANY_SOURCE) {
    actual_src = ezt_get_global_rank(CUR_INDEX, comm, src);
  }
  assert(actual_src!= -1);

  struct mpi_pers_req_t __attribute__((unused)) *req = __pers_init(CURRENT, mpi_req_recv, buffer,
								   actual_src, dest, len, tag, mpi_req);

  DECLARE_CUR_THREAD(cur_thread);
}

void handle_mpi_start()
{
  FUNC_NAME;
  DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);
  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

  app_ptr mpi_req = (app_ptr)GET_PARAM(CUR_EV, 1);
  struct mpi_pers_req_t* req = __pers_start(CURRENT,
					    CUR_RANK,
					    mpi_req);
  assert(req);

  if(req->type == mpi_req_send)
    /* todo: mpi communicator */
    __mpi_send_generic( thread_id, CUR_RANK, req->dest, req->len, req->tag, req->mpi_req, p_info->__MPI_COMM_WORLD);
  else
    __mpi_start_recv_generic( thread_id, req->src, CUR_RANK, req->len, req->tag, req->mpi_req, p_info->__MPI_COMM_WORLD);

  /* todo: add a send event here */
  MPI_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, "STV_MPI_Overlap");
}

/* return 1 if comm_id already exists in comm_list
 * for debugging purpose only
 */
static int __find_communicator(app_ptr comm_id, struct ezt_list_t *comm_list)
{
  struct ezt_list_token_t *token;
  ezt_list_foreach(comm_list, token) {
    struct ezt_mpi_comm* p_comm = (struct ezt_mpi_comm*) token->data;
    assert(p_comm);
    if(p_comm->comm_id == comm_id) {
      /* we found the communicator ! */
      return 1;
    }
    return 0;
  }

}

/* create a new mpi communicator */
void handle_mpi_new_comm()
{
  FUNC_NAME;

  /* get the communicator id (eg. MPI_COMM_WORLD) and the number of processes */
  app_ptr comm_id = (app_ptr)GET_PARAM(CUR_EV, 1);
  int comm_size = GET_PARAM(CUR_EV, 2);

  INIT_MPI_PROCESS_INFO(GET_PROCESS_INFO(CUR_INDEX), p_info);

  /* allocate a new ezt_mpi_comm and fill it */
  struct ezt_mpi_comm* p_comm = malloc(sizeof(struct ezt_mpi_comm));
  p_comm->comm_id = comm_id;
  p_comm->comm_size = comm_size;
  p_comm->ranks = malloc(sizeof(int)*comm_size);

#if EZTRACE_DEBUG
  if(__find_communicator(comm_id, &p_info->communicators)) {
    fprintf(stderr, "[%d] Error: trying to add a communicator (%d) that already exists!\n", comm_id);
    abort();
  }
#endif

  /* for each rank in the new communicator, translate to the global rank
   * (the corresponding rank in MPI_COMM_WORLD) */
  int i;
  for(i=0; i<comm_size; i++) {
      wait_for_an_event(CUR_INDEX, FUT_MPI_NEW_COMM_Info);
      p_comm->ranks[i] = GET_PARAM(CUR_EV, 1);
  }

  ezt_list_add(&p_info->communicators, &p_comm->token);
  p_comm->token.data = p_comm;
}

int
eztrace_convert_mpi_init()
{
  if(get_mode() == EZTRACE_CONVERT) {
    /* Process send a message */
    addEntityValue("STV_MPI_Send", "ST_Thread", "Sending", GTG_ORANGE);
    addEntityValue("STV_MPI_Sendrecv", "ST_Thread", "SendRecv", GTG_ORANGE);
    /* Process receive a message */
    addEntityValue("STV_MPI_Recv", "ST_Thread", "Receiving", GTG_PURPLE);
    addEntityValue("STV_MPI_Waitany", "ST_Thread", "MPI_Waitany", GTG_PURPLE);
    addEntityValue("STV_MPI_Waitall", "ST_Thread", "MPI_Waitall", GTG_PURPLE);

    addEntityValue("STV_MPI_Barrier", "ST_Thread", "MPI_Barrier", GTG_PINK);
    addEntityValue("STV_MPI_BCast", "ST_Thread", "MPI_BCast", GTG_PINK);
    addEntityValue("STV_MPI_Gather", "ST_Thread", "MPI_Gather", GTG_PINK);
    addEntityValue("STV_MPI_Scatter", "ST_Thread", "MPI_Scatter", GTG_PINK);
    addEntityValue("STV_MPI_Allgather", "ST_Thread", "MPI_Allgather", GTG_PINK);
    addEntityValue("STV_MPI_Alltoall", "ST_Thread", "MPI_Alltoall", GTG_PINK);
    addEntityValue("STV_MPI_Reduce", "ST_Thread", "MPI_Reduce", GTG_PINK);
    addEntityValue("STV_MPI_Allreduce", "ST_Thread", "MPI_Allreduce", GTG_PINK);
    addEntityValue("STV_MPI_reduce_scatter", "ST_Thread", "MPI_Reduce_scatter", GTG_PINK);
    addEntityValue("STV_MPI_Scan", "ST_Thread", "MPI_Scan", GTG_PINK);

    addEntityValue("STV_MPI_Overlap", "ST_Thread", "MPI_Overlap", GTG_YELLOW);
    addEntityValue("STV_MPI_Probe", "ST_Thread", "MPI_Probe", GTG_PURPLE);

    addLinkType ("L_MPI_Coll", "MPI collective communication", "CT_Program", "CT_Thread", "CT_Thread");
    addLinkType ("L_MPI_P2P", "MPI point to point communication", "CT_Program", "CT_Thread", "CT_Thread");
    addLinkType ("L_MPI_SPAWN", "MPI SPAWN", "CT_Program", "CT_Thread", "CT_Thread");

    addEventType ("E_MPI_CommSend", "CT_Thread", "MPI Send");
    addEventType ("E_MPI_CommRecv", "CT_Thread", "MPI Recv");

    addEventType ("E_MPI_EndComm", "CT_Thread", "End of an MPI communication");

    addEventType ("E_MPI_Probe_success", "CT_Thread", "MPI_Probe");
    addEventType ("E_MPI_Iprobe_success", "CT_Thread", "MPI_IProbe success");
    addEventType ("E_MPI_Iprobe_failed", "CT_Thread", "MPI_IProbe failed");
  }

  ezt_list_new(&spawn_list);

  init_mpi_pers_messages();
  init_mpi_p2p_messages();
  init_mpi_coll_messages();
  init_mpi_stats();
  return 0;
}

/* return 1 if the event was handled */
int
handle_mpi_events(struct fxt_ev_64 *ev)
{
  switch (ev->code)
    {
    case FUT_MPI_INIT:
      handle_mpi_init ();
      break;
    case FUT_MPI_NEW_COMM:
      handle_mpi_new_comm ();
      break;
    case FUT_MPI_START_SEND:
      nb_mpi_calls[MPI_SEND_ID]++;
      handle_mpi_start_send ();
      break;
    case FUT_MPI_STOP_SEND:
      handle_mpi_stop_send ();
      break;
    case FUT_MPI_START_RECV:
      nb_mpi_calls[MPI_RECV_ID]++;
      handle_mpi_start_recv ();
      break;
    case FUT_MPI_STOP_RECV:
      handle_mpi_stop_recv ();
      break;

    case FUT_MPI_START_SENDRECV:
      nb_mpi_calls[MPI_SENDRECV_ID]++;
      handle_mpi_start_sendrecv ();
      break;
    case FUT_MPI_STOP_SENDRECV:
      handle_mpi_stop_sendrecv ();
      break;
    case FUT_MPI_START_SENDRECV_REPLACE:
      nb_mpi_calls[MPI_SENDRECV_REPLACE_ID]++;
      handle_mpi_start_sendrecv_replace ();
      break;
    case FUT_MPI_STOP_SENDRECV_REPLACE:
      handle_mpi_stop_sendrecv_replace ();
      break;

    case FUT_MPI_START_BSEND:
      nb_mpi_calls[MPI_BSEND_ID]++;
      handle_mpi_start_bsend ();
      break;
    case FUT_MPI_STOP_BSEND:
      handle_mpi_stop_bsend ();
      break;

    case FUT_MPI_START_SSEND:
      nb_mpi_calls[MPI_SSEND_ID]++;
      handle_mpi_start_ssend ();
      break;
    case FUT_MPI_STOP_SSEND:
      handle_mpi_stop_ssend ();
      break;

    case FUT_MPI_START_RSEND:
      nb_mpi_calls[MPI_RSEND_ID]++;
      handle_mpi_start_rsend ();
      break;
    case FUT_MPI_STOP_RSEND:
      handle_mpi_stop_rsend ();
      break;

    case FUT_MPI_ISEND:
      nb_mpi_calls[MPI_ISEND_ID]++;
      handle_mpi_isend ();
      break;
    case FUT_MPI_IBSEND:
      nb_mpi_calls[MPI_IBSEND_ID]++;
      handle_mpi_ibsend ();
      break;
    case FUT_MPI_ISSEND:
      nb_mpi_calls[MPI_ISSEND_ID]++;
      handle_mpi_issend ();
      break;
    case FUT_MPI_IRSEND:
      nb_mpi_calls[MPI_IRSEND_ID]++;
      handle_mpi_irsend ();
      break;
    case FUT_MPI_IRECV:
      nb_mpi_calls[MPI_IRECV_ID]++;
      handle_mpi_irecv ();
      break;

    case FUT_MPI_START_PUT:
      nb_mpi_calls[MPI_PUT_ID]++;
      handle_mpi_start_put();
      break;
    case FUT_MPI_STOP_PUT:
      handle_mpi_stop_put();
      break;
    case FUT_MPI_START_GET:
      nb_mpi_calls[MPI_GET_ID]++;
      handle_mpi_start_get();
      break;
    case FUT_MPI_STOP_GET:
      handle_mpi_stop_get();
      break;

    case FUT_MPI_START_WAIT:
      nb_mpi_calls[MPI_WAIT_ID]++;
      handle_mpi_start_wait();
      break;
    case FUT_MPI_STOP_WAIT:
      handle_mpi_stop_wait();
      break;
    case FUT_MPI_START_WAITANY:
      nb_mpi_calls[MPI_WAITANY_ID]++;
      handle_mpi_start_waitany();
      break;
    case FUT_MPI_STOP_WAITANY:
      handle_mpi_stop_waitany();
      break;
    case FUT_MPI_START_WAITALL:
      nb_mpi_calls[MPI_WAITALL_ID]++;
      handle_mpi_start_waitall();
      break;
    case FUT_MPI_STOP_WAITALL:
      handle_mpi_stop_waitall(NULL);
      break;
    case FUT_MPI_TEST_SUCCESS:
      nb_mpi_calls[MPI_TEST_ID]++;
      handle_mpi_test_success();
      break;

    case FUT_MPI_START_PROBE:
      nb_mpi_calls[MPI_PROBE_ID]++;
      handle_mpi_start_probe();
      break;
    case FUT_MPI_STOP_PROBE:
      handle_mpi_stop_probe();
      break;
    case FUT_MPI_IPROBE_SUCCESS:
      nb_mpi_calls[MPI_IPROBE_ID]++;
      handle_mpi_iprobe_success();
      break;
    case FUT_MPI_IPROBE_FAILED:
      handle_mpi_iprobe_failed();
      break;

    case FUT_MPI_START_BCast:
      nb_mpi_calls[MPI_BCAST_ID]++;
      handle_mpi_start_BCast();
      break;
    case FUT_MPI_START_Gather:
      nb_mpi_calls[MPI_GATHER_ID]++;
      handle_mpi_start_Gather();
      break;
    case FUT_MPI_START_Gatherv:
      nb_mpi_calls[MPI_GATHERV_ID]++;
      handle_mpi_start_Gatherv();
      break;
    case FUT_MPI_START_Scatter:
      nb_mpi_calls[MPI_SCATTER_ID]++;
      handle_mpi_start_Scatter();
      break;
    case FUT_MPI_START_Scatterv:
      nb_mpi_calls[MPI_SCATTERV_ID]++;
      handle_mpi_start_Scatterv();
      break;
    case FUT_MPI_START_Allgather:
      nb_mpi_calls[MPI_ALLGATHER_ID]++;
      handle_mpi_start_Allgather();
      break;
    case FUT_MPI_START_Allgatherv:
      nb_mpi_calls[MPI_ALLGATHERV_ID]++;
      handle_mpi_start_Allgatherv();
      break;
    case FUT_MPI_START_Alltoall:
      nb_mpi_calls[MPI_ALLTOALL_ID]++;
      handle_mpi_start_Alltoall();
      break;
    case FUT_MPI_START_Alltoallv:
      nb_mpi_calls[MPI_ALLTOALLV_ID]++;
      handle_mpi_start_Alltoallv();
      break;
    case FUT_MPI_START_Reduce:
      nb_mpi_calls[MPI_REDUCE_ID]++;
      handle_mpi_start_Reduce();
      break;
    case FUT_MPI_START_Allreduce:
      nb_mpi_calls[MPI_ALLREDUCE_ID]++;
      handle_mpi_start_Allreduce();
      break;
    case FUT_MPI_START_Reduce_scatter:
      nb_mpi_calls[MPI_REDUCE_SCATTER_ID]++;
      handle_mpi_start_Reduce_scatter();
      break;
    case FUT_MPI_START_Scan:
      nb_mpi_calls[MPI_SCAN_ID]++;
      handle_mpi_start_Scan();
      break;
    case FUT_MPI_START_BARRIER:
      nb_mpi_calls[MPI_BARRIER_ID]++;
      handle_mpi_start_barrier();
      break;

    case FUT_MPI_STOP_BCast          : handle_mpi_stop_BCast(); break;
    case FUT_MPI_STOP_Gather         : handle_mpi_stop_Gather(); break;
    case FUT_MPI_STOP_Gatherv        : handle_mpi_stop_Gatherv(); break;
    case FUT_MPI_STOP_Scatter        : handle_mpi_stop_Scatter(); break;
    case FUT_MPI_STOP_Scatterv       : handle_mpi_stop_Scatterv(); break;
    case FUT_MPI_STOP_Allgather      : handle_mpi_stop_Allgather(); break;
    case FUT_MPI_STOP_Allgatherv     : handle_mpi_stop_Allgatherv(); break;
    case FUT_MPI_STOP_Alltoall       : handle_mpi_stop_Alltoall(); break;
    case FUT_MPI_STOP_Alltoallv      : handle_mpi_stop_Alltoallv(); break;
    case FUT_MPI_STOP_Reduce         : handle_mpi_stop_Reduce(); break;
    case FUT_MPI_STOP_Allreduce      : handle_mpi_stop_Allreduce(); break;
    case FUT_MPI_STOP_Reduce_scatter : handle_mpi_stop_Reduce_scatter(); break;
    case FUT_MPI_STOP_Scan           : handle_mpi_stop_Scan(); break;
    case FUT_MPI_STOP_BARRIER        : handle_mpi_stop_barrier(); break;

    case FUT_MPI_SPAWN               : handle_mpi_spawn(); break;
    case FUT_MPI_SPAWNED             : handle_mpi_spawned(); break;

    case FUT_MPI_SEND_INIT:
      handle_mpi_send_init();
      break;
    case FUT_MPI_BSEND_INIT:
      handle_mpi_bsend_init();
      break;
    case FUT_MPI_RSEND_INIT:
      handle_mpi_rsend_init();
      break;
    case FUT_MPI_SSEND_INIT:
      handle_mpi_ssend_init();
      break;
    case FUT_MPI_RECV_INIT:
      handle_mpi_recv_init();
      break;
    case FUT_MPI_START:
      nb_mpi_calls[MPI_START_ID]++;
      handle_mpi_start();
      break;

    default:
      return 0;
    }
  return 1;
}

int
handle_mpi_stats(struct fxt_ev_64 *ev)
{
  recording_stats = 1;
  return handle_mpi_events(ev);
}

#define FORMAT_BYTES(nb_bytes)						\
  ((uint64_t)nb_bytes<1024?"B":						\
   ((uint64_t)nb_bytes<1024*1024?"KB":					\
    ((uint64_t)nb_bytes<1024*1024*1024?"MB":				\
     ((uint64_t)nb_bytes<(uint64_t)1024*1024*1024*1024?"GB":		\
      ((uint64_t)nb_bytes<(uint64_t)1024*1024*1024*1024*1024?"TB":	\
       "PB")))))

float VALUE_BYTES(uint64_t nb_bytes){
  int i;
  uint64_t div=1;
  for(i=0; i<6; i++) {
    if((nb_bytes/div)<1024)
      return (nb_bytes/(double)div);
    div*=1024;
  }
  return (float)nb_bytes;
}

void print_mpi_stats() {
  printf ( "\nMPI:\n");
  printf   ( "---\n");

  /* todo:
   * add:
   *  - communication pattern (matrix)
   *  - min/max/average message length
   *  - min/max/average communication duration
   */

  int i;
  /* Print the of mpi_send, isend, issend, bcast, gather, scatter etc. */
  for(i=0; i<MPI_ID_SIZE; i++) {
    /* don't print anything if the function was not called */
    if(nb_mpi_calls[i]) {
      printf("\t");

      switch(i) {
      case MPI_SEND_ID             : printf("MPI_SEND             :"); break;
      case MPI_RECV_ID             : printf("MPI_RECV             :"); break;
      case MPI_BSEND_ID            : printf("MPI_BSEND            :"); break;
      case MPI_SSEND_ID            : printf("MPI_SSEND            :"); break;
      case MPI_RSEND_ID            : printf("MPI_RSEND            :"); break;
      case MPI_ISEND_ID            : printf("MPI_ISEND            :"); break;
      case MPI_IBSEND_ID           : printf("MPI_IBSEND           :"); break;
      case MPI_ISSEND_ID           : printf("MPI_ISSEND           :"); break;
      case MPI_IRSEND_ID           : printf("MPI_IRSEND           :"); break;
      case MPI_IRECV_ID            : printf("MPI_IRECV            :"); break;
      case MPI_SENDRECV_ID         : printf("MPI_SENDRECV         :"); break;
      case MPI_SENDRECV_REPLACE_ID : printf("MPI_SENDRECV_REPLACE :"); break;
      case MPI_START_ID            : printf("MPI_START            :"); break;
      case MPI_STARTALL_ID         : printf("MPI_STARTALL         :"); break;
      case MPI_WAIT_ID             : printf("MPI_WAIT             :"); break;
      case MPI_TEST_ID             : printf("MPI_TEST             :"); break;
      case MPI_WAITANY_ID          : printf("MPI_WAITANY          :"); break;
      case MPI_TESTANY_ID          : printf("MPI_TESTANY          :"); break;
      case MPI_WAITALL_ID          : printf("MPI_WAITALL          :"); break;
      case MPI_TESTALL_ID          : printf("MPI_TESTALL          :"); break;
      case MPI_WAITSOME_ID         : printf("MPI_WAITSOME         :"); break;
      case MPI_TESTSOME_ID         : printf("MPI_TESTSOME         :"); break;
      case MPI_PROBE_ID            : printf("MPI_PROBE            :"); break;
      case MPI_IPROBE_ID           : printf("MPI_IPROBE           :"); break;
      case MPI_BARRIER_ID          : printf("MPI_BARRIER          :"); break;
      case MPI_BCAST_ID            : printf("MPI_BCAST            :"); break;
      case MPI_GATHER_ID           : printf("MPI_GATHER           :"); break;
      case MPI_GATHERV_ID          : printf("MPI_GATHERV          :"); break;
      case MPI_SCATTER_ID          : printf("MPI_SCATTER          :"); break;
      case MPI_SCATTERV_ID         : printf("MPI_SCATTERV         :"); break;
      case MPI_ALLGATHER_ID        : printf("MPI_ALLGATHER        :"); break;
      case MPI_ALLGATHERV_ID       : printf("MPI_ALLGATHERV       :"); break;
      case MPI_ALLTOALL_ID         : printf("MPI_ALLTOALL         :"); break;
      case MPI_ALLTOALLV_ID        : printf("MPI_ALLTOALLV        :"); break;
      case MPI_REDUCE_ID           : printf("MPI_REDUCE           :"); break;
      case MPI_ALLREDUCE_ID        : printf("MPI_ALLREDUCE        :"); break;
      case MPI_REDUCE_SCATTER_ID   : printf("MPI_REDUCE_SCATTER   :"); break;
      case MPI_SCAN_ID             : printf("MPI_SCAN             :"); break;
      case MPI_GET_ID              : printf("MPI_GET              :"); break;
      case MPI_PUT_ID              : printf("MPI_PUT              :"); break;
      }

      printf("%d calls\n", nb_mpi_calls[i]);
    }
  }


  print_p2p_stats();

  printf("\n");
  mpi_stats_plot_message_size();
  printf("\n");
}

struct eztrace_convert_module mpi_module;
void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  char *tmp;
  tmp = getenv("UNUSED_MPI");
  if(tmp != NULL) _UNUSED_MPI = MAX_LEVEL_MPI>atoi(tmp)?atoi(tmp):MAX_LEVEL_MPI;
  else _UNUSED_MPI = 0;

  printf("_UNUSED_MPI in convert = %i\n",_UNUSED_MPI);
  mpi_module.api_version = EZTRACE_API_VERSION;
  mpi_module.init = eztrace_convert_mpi_init;
  mpi_module.handle = handle_mpi_events;
  mpi_module.handle_stats = handle_mpi_stats;
  mpi_module.print_stats = print_mpi_stats;
  mpi_module.module_prefix = MPI_EVENTS_ID;
  asprintf(&mpi_module.name, "mpi");
  asprintf(&mpi_module.description, "Module for MPI functions");
  mpi_module.token.data = &mpi_module;
  eztrace_convert_register_module(&mpi_module);

  int i;
  for(i=0; i<MPI_ID_SIZE; i++)
    nb_mpi_calls[i] = 0;
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
}

#endif	/* USE_MPI */
