/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <assert.h>
#include "eztrace_stats_core.h"
#include "eztrace_stats_mpi.h"
#include "mpi_ev_codes.h"
#include "eztrace_list.h"
#include "eztrace_convert.h"
#include "eztrace_convert_mpi.h"
#include "eztrace_convert_mpi_p2p.h"
#include "eztrace_hierarchical_array.h"
#include "eztrace_array.h"

struct mpi_p2p_stat_t {
  int nb_messages;
  /* size of sent messages */
  struct stat_uint64_t_counter_t size;

  /* time spent sending messages */
  struct stat_double_counter_t scomm_duration;
  /* time spent computing while sending messages */
  struct stat_double_counter_t soverlap_duration;
  /* time spent in MPI_Send or waiting for a Isend to complete */
  struct stat_double_counter_t swait_duration;

  /* time spent receiving messages */
  struct stat_double_counter_t rcomm_duration;
  /* time spending computing while waiting for an incoming message */
  struct stat_double_counter_t roverlap_duration;
  /* time spent in MPI_Recv or waiting for a Irecv to complete */
  struct stat_double_counter_t rwait_duration;
};

struct __mpi_stats_matrix_item {
  uint64_t total_len;
  int nb_messages;
};

struct __mpi_stats_freq_item {
  int len;
  int nb_occur;
};

struct eztrace_array_t __mpi_stats_freq;
struct __mpi_stats_matrix_item **__mpi_stats_comm_matrix = NULL;

/* find a len in the frequency table */
static struct __mpi_stats_freq_item*
__find_freq_item(int len)
{
  struct __mpi_stats_freq_item* ret = NULL;
  unsigned i;
  for(i=0; i<__mpi_stats_freq.nb_items; i++) {
    ret = ITH_VALUE(i, &__mpi_stats_freq);
    if(ret->len == len)
      return ret;
  }

  /* can't find the specified len, allocate a new item */
  ret = eztrace_array_new_value(&__mpi_stats_freq);
  ret->len = len;
  ret->nb_occur = 0;
  return ret;
}

/* update the communication matrix and the frequency table for p2p messages */
static void __update_p2p_message_stats(int src, int dest, int len, int __attribute__((unused)) tag)
{
  __mpi_stats_comm_matrix[src][dest].nb_messages++;
  __mpi_stats_comm_matrix[src][dest].total_len += len;

  struct __mpi_stats_freq_item* freq_item = __find_freq_item(len);
  assert(freq_item->len == len);
  freq_item->nb_occur++;
}


static void __print_stat_double_counter(struct stat_double_counter_t *counter, int count)
{
  printf("\tmin: %lf", counter->min);
  printf("\tmax: %lf", counter->max);
  printf("\taverage: %lf", counter->sum / count);
  printf("\ttotal: %lf", counter->sum);
}

static void __print_stat_int_counter(struct stat_uint64_t_counter_t *counter, int count)
{
  printf("\tmin: %lld", counter->min);
  printf("\tmax: %lld", counter->max);
  printf("\taverage: %lld", counter->sum / count);
  printf("\ttotal: %lld", counter->sum);
}


static void __init_mpi_p2p_stat_t(struct mpi_p2p_stat_t* counter)
{
  counter->nb_messages = 0;
  __init_stat_counter(uint64_t, &counter->size);

  __init_stat_counter(double, &counter->scomm_duration);
  __init_stat_counter(double, &counter->soverlap_duration);
  __init_stat_counter(double, &counter->swait_duration);

  __init_stat_counter(double, &counter->rcomm_duration);
  __init_stat_counter(double, &counter->roverlap_duration);
  __init_stat_counter(double, &counter->rwait_duration);
}

static void __p2p_stats_reduce_recurse(unsigned depth, int rank, p_eztrace_container p_cont)
{
  assert(p_cont);

  struct mpi_p2p_stat_t* counter = hierarchical_array_new_item(p_cont, MPI_STATS_P2P_ID);
  assert(counter);
  __init_mpi_p2p_stat_t(counter);

  unsigned i;
  /* compute the children stats */
  for(i=0; i<p_cont->nb_children; i++) {
    /* call recursively the function */
    __p2p_stats_reduce_recurse(depth+1, rank, p_cont->children[i]);
    struct hierarchical_array* child_array = hierarchical_array_find(MPI_STATS_P2P_ID, p_cont->children[i]);
    assert(child_array);
    assert(child_array->nb_items);

    /* add the children stats to the container stats */
    struct mpi_p2p_stat_t*child_counter = ITH_ITEM(0, child_array);
    counter->nb_messages += child_counter->nb_messages;

    __reduce_stat_counter(uint64_t, &counter->size, &child_counter->size);

    __reduce_stat_counter(double, &counter->scomm_duration, &child_counter->scomm_duration);
    __reduce_stat_counter(double, &counter->soverlap_duration, &child_counter->soverlap_duration);
    __reduce_stat_counter(double, &counter->swait_duration, &child_counter->swait_duration);

    __reduce_stat_counter(double, &counter->rcomm_duration, &child_counter->rcomm_duration);
    __reduce_stat_counter(double, &counter->roverlap_duration, &child_counter->roverlap_duration);
    __reduce_stat_counter(double, &counter->rwait_duration, &child_counter->rwait_duration);
  }

  /* compute statistics for the current container */
  unsigned index;
  /* sent messages */
  struct hierarchical_array* array = hierarchical_array_find(MPI_P2P_ISEND_ID, p_cont);
  assert(array);
  for(index = 0; index<array->nb_items; index++) {
    struct p2p_msg_event *msg = NULL;
    msg = ITH_ITEM(index, array);
    assert(msg);

    counter->nb_messages ++;
    __update_stat_counter(uint64_t, &counter->size, (uint64_t)msg->msg->len);
    __update_p2p_message_stats(msg->msg->src, msg->msg->dest, msg->msg->len, msg->msg->tag);

    /* compute communication durations */
    __update_stat_counter(double, &counter->scomm_duration,
			  (double) (msg->msg->times[stop_send] - msg->msg->times[start_send]) / 1000000);

    __update_stat_counter(double, &counter->soverlap_duration,
			  (double) (msg->msg->times[start_swait] - msg->msg->times[start_send]) / 1000000);

    __update_stat_counter(double, &counter->swait_duration,
			  (double) (msg->msg->times[stop_send] - msg->msg->times[start_swait]) / 1000000);
  }

  /* received messages */
  array = hierarchical_array_find(MPI_P2P_IRECV_ID, p_cont);
  assert(array);
  for(index = 0; index<array->nb_items; index++) {
    struct p2p_msg_event *msg = NULL;
    msg = ITH_ITEM(index, array);
    assert(msg);

    /* compute communication durations */
    __update_stat_counter(double, &counter->rcomm_duration,
			  (double) (msg->msg->times[stop_recv] - msg->msg->times[start_recv]) / 1000000);

    __update_stat_counter(double, &counter->roverlap_duration,
			  (double) (msg->msg->times[start_rwait] - msg->msg->times[start_recv]) / 1000000);

    __update_stat_counter(double, &counter->rwait_duration,
			  (double) (msg->msg->times[stop_recv] - msg->msg->times[start_rwait]) / 1000000);
  }

}

static void __p2p_stats_print_recurse(unsigned depth, p_eztrace_container p_cont)
{
  assert(p_cont);

  struct hierarchical_array* array = hierarchical_array_find(MPI_STATS_P2P_ID, p_cont);
  assert(array);

  struct mpi_p2p_stat_t* counter = ITH_ITEM(0, array);
  assert(counter);

  unsigned i;
  if(counter->nb_messages) {
    /* Print the current container stats */
    for(i=0;i<depth; i++)
      printf("   ");
    printf("%s -- \t%d messages sent\n", p_cont->name, counter->nb_messages);

    for(i=0;i<depth; i++)
      printf("   ");
    printf("\tSize of messages (byte):");
    __print_stat_int_counter(&counter->size, counter->nb_messages);
    printf("\n");

    /* sent messages */
    for(i=0;i<depth; i++)
      printf("   ");
    printf("\tTime spent sending messages (ms):");
    __print_stat_double_counter(&counter->scomm_duration, counter->nb_messages);
    printf("\n");

    for(i=0;i<depth; i++)
      printf("   ");
    printf("\tTime spent computing while sending messages (ms):");
    __print_stat_double_counter(&counter->soverlap_duration, counter->nb_messages);
    printf("\n");

    for(i=0;i<depth; i++)
      printf("   ");
    printf("\tTime spent in MPI_Send or waiting for a Isend to complete (ms):");
    __print_stat_double_counter(&counter->swait_duration, counter->nb_messages);
    printf("\n");

    /* received messages */
    for(i=0;i<depth; i++)
      printf("   ");
    printf("\tTime spent receiving messages (ms):");
    __print_stat_double_counter(&counter->rcomm_duration, counter->nb_messages);
    printf("\n");

    for(i=0;i<depth; i++)
      printf("   ");
    printf("\tTime spent computing while receiving messages (ms):");
    __print_stat_double_counter(&counter->roverlap_duration, counter->nb_messages);
    printf("\n");

    for(i=0;i<depth; i++)
      printf("   ");
    printf("\tTime spent in MPI_Recv or waiting for a Irecv to complete (ms):");
    __print_stat_double_counter(&counter->rwait_duration, counter->nb_messages);
    printf("\n");

    /* call the container children so that they print their values */
    for(i=0; i<p_cont->nb_children; i++) {
      __p2p_stats_print_recurse(depth+1, p_cont->children[i]);
    }
  }

}

/* compute point to point messages statistics */
static void __p2p_stats_reduce()
{
  int rank;
  eztrace_array_create(&__mpi_stats_freq, sizeof(struct __mpi_stats_freq_item), 32);

  __mpi_stats_comm_matrix = malloc(sizeof(struct __mpi_stats_matrix_item*) * NB_TRACES);
  for(rank=0; rank<NB_TRACES; rank++) {
    __mpi_stats_comm_matrix[rank] = malloc(sizeof(struct __mpi_stats_matrix_item) * NB_TRACES);
    int i;
    for(i=0; i<NB_TRACES; i++) {
      __mpi_stats_comm_matrix[rank][i].total_len = 0;
      __mpi_stats_comm_matrix[rank][i].nb_messages = 0;
    }
  }

  for(rank=0; rank<NB_TRACES; rank++) {
    p_eztrace_container p_cont = GET_PROCESS_CONTAINER(rank);
    __p2p_stats_reduce_recurse(0, rank, p_cont);
  }
}

/* print point to point messages statistics */
static void __p2p_stats_print()
{
  int rank;
  for(rank=0; rank<NB_TRACES; rank++) {
    p_eztrace_container p_cont = GET_PROCESS_CONTAINER(rank);
    __p2p_stats_print_recurse(0, p_cont);
  }
}

/* compute point to point messages statistics and print them */
void print_p2p_stats()
{
  __p2p_stats_reduce();
  __p2p_stats_print();

  mpi_stats_dump();
}

/* initialize MPI statistics module */
void init_mpi_stats() {
  hierarchical_array_attach(MPI_STATS_P2P_ID, sizeof(struct mpi_p2p_stat_t));

  /* todo: do the same for collective */
}

/* dump to disk the list of messages */
void mpi_stats_dump()
{
  char* env_str = getenv("EZTRACE_MPI_DUMP_MESSAGES");

  if(env_str) {
    char* path;
    asprintf(&path, "%s/%s_eztrace_message_dump", eztrace_stats_get_output_dir(), getenv("USER"));
    FILE *f = fopen(path, "w");
    if(!f)
      perror("Error while dumping messages");

    __print_p2p_messages(f);
    int ret = fclose(f);
    if(ret)
      perror("Error while dumping messages (fclose)");

    printf("\nMPI messages dumped in %s\n", path);
    free(path);
  }
}


void mpi_stats_plot_message_size(){
  char* path;
  asprintf(&path, "%s/data.dat", eztrace_stats_get_output_dir());
    FILE *out = fopen(path, "w+");

  struct __mpi_stats_freq_item* ret = NULL;
  unsigned i;
  fprintf(out, "#Message_length  #Number_of_messages\n");
  for(i=0; i<__mpi_stats_freq.nb_items; i++) {
    ret = ITH_VALUE(i, &__mpi_stats_freq);
    fprintf(out, "%d\t%d\n", ret->len, ret->nb_occur);
  }
  fclose(out);

  free(path);
  asprintf(&path, "%s/message_size.gp", eztrace_stats_get_output_dir());
  out = fopen(path, "w+");
  fprintf(out, "set xlabel \"message size (B)\"\n");
  fprintf(out, "set ylabel \"number of messages\"\n");
  fprintf(out, "plot \"%s/data.dat\" with lines\n", eztrace_stats_get_output_dir());
  fclose(out);

  printf("\t%s was created\n", path);
  free(path);
}
