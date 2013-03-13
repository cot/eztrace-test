#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <assert.h>
#include <stdio.h>
#include "mpi_ev_codes.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_hierarchical_array.h"
#include "eztrace_convert_mpi_coll.h"


void init_mpi_coll_messages()
{
  hierarchical_array_attach(MPI_COLL_ID, sizeof(struct mpi_coll_msg_t));
  hierarchical_array_attach(MPI_COLL_ENTER_ID, sizeof(struct coll_msg_event));
  hierarchical_array_attach(MPI_COLL_LEAVE_ID, sizeof(struct coll_msg_event));
}

void print_coll_stats();

void __print_coll_message(FILE*stream, struct mpi_coll_msg_t *msg)
{
    fprintf(stream, "[%p]\ttype %d, len = %d\n", msg, msg->type, msg->len);
    int i;
    for(i=0; i<msg->comm_size; i++) {
      fprintf(stream, "\t");
      fprintf(stream, "[%s] enter %lld\tleave: %lld\n", msg->thread_ids[i],
	      msg->times[i][start_coll], msg->times[i][stop_coll]);
    }
    fprintf(stream, "\n");
}

void __print_coll_messages_recurse(FILE*stream, unsigned depth, p_eztrace_container p_cont);
void __print_coll_messages(FILE*stream)
{
  struct hierarchical_array* array = hierarchical_array_find(MPI_COLL_ID, NULL);
  struct mpi_coll_msg_t* msg = NULL;
  unsigned index;

  for(index = 0; index< (array->nb_items); index++) {
    msg = ITH_ITEM(index, array);
    __print_coll_message(stream, msg);
  }
}

static struct mpi_coll_msg_t* __new_coll_message(enum coll_type_t type,
						 int comm_size,
						 int len)
{
  struct mpi_coll_msg_t* msg = hierarchical_array_new_item(NULL, MPI_COLL_ID);
  assert(msg);

  msg->type = type;
  msg->len = len;
  msg->comm_size = comm_size;
  int i;
  msg->times = malloc(sizeof(uint64_t*) * comm_size);
  msg->thread_ids = malloc(sizeof(char*) * comm_size);
  msg->link_value = malloc(sizeof(char**) * comm_size);
  msg->link_id = malloc(sizeof(char**) * comm_size);
  for(i=0; i<comm_size; i++) {
    int j;
    msg->thread_ids[i] = NULL;
    msg->link_value[i] = malloc(sizeof(char*)* comm_size);
    msg->link_id[i] = malloc(sizeof(char*)* comm_size);
    for(j=0; j<comm_size; j++) {
      CREATE_COLL_LINK_VALUE(msg, i, j);
      CREATE_COLL_MSG_ID(msg, i, j);
    }

    msg->times[i] = malloc(sizeof(uint64_t)* COLL_NB_TIMES);
    for(j=0; j<COLL_NB_TIMES; j++)
      msg->times[i][j] = 0;
  }

  return msg;
}


struct mpi_coll_msg_t*
__mpi_find_coll_message(enum coll_type_t type,
			int __attribute__((unused)) comm_size, int my_rank, int len,
			int time_id)
{
  struct hierarchical_array* array = hierarchical_array_find(MPI_COLL_ID, NULL);
  struct mpi_coll_msg_t* msg = NULL;
  int index;

  for(index = 0; index <array->nb_items; index++) {
    msg = ITH_ITEM(index, array);

    /* todo: take the communicator into account ? */
    if((msg->type == type) &&
       (msg->len == len) &&
       (time_id<0 ||!msg->times[my_rank][time_id])) {
      return msg;
    }
  }
  return NULL;
}

struct mpi_coll_msg_t*
__enter_coll(uint64_t time,
	     enum coll_type_t type,
	     int comm_size,
	     int my_rank,
	     int len,
	     char* thread_id)
{
  struct mpi_coll_msg_t* msg = __mpi_find_coll_message(type, comm_size, my_rank, len, start_coll);

  if ( !msg) {
    msg = __new_coll_message(type, comm_size, len);
  }

  /* todo: record_event(time, type, msg) */
  msg->times[my_rank][start_coll] = time;
  msg->thread_ids[my_rank] = thread_id;
  return msg;
}

struct mpi_coll_msg_t*
__leave_coll(uint64_t time,
	     enum coll_type_t type,
	     int comm_size,
	     int my_rank,
	     int len,
	     char* thread_id)
{
  struct mpi_coll_msg_t* msg = __mpi_find_coll_message(type, comm_size, my_rank, len, stop_coll);

  assert (msg);

  /* todo: record_event(time, type, msg) */
  msg->times[my_rank][stop_coll] = time;
  assert(msg->thread_ids[my_rank] == thread_id);

  return msg;
}
