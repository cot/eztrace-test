/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "eztrace_stats_core.h"

#define MAX_COUNTERS 16

struct eztrace_counter{
  eztrace_counter_id id;
  unsigned  nb_values;
  double* values;
  char** value_names;
  char* name;
};

/* todo: make this dynamic */
static struct eztrace_counter root_counters[MAX_COUNTERS];
static unsigned nb_counters = 0;

char* __output_stats_dir = NULL;
void eztrace_stats_set_output_dir(const char*path)
{
  assert(path);
  fprintf(stderr, "setting output dir to %s\n", path);
  asprintf(&__output_stats_dir, "%s", path);
  int ret = mkdir(path, S_IRWXU);
  if(ret < 0 && errno != EEXIST)
    perror("mkdir");
}

const char* eztrace_stats_get_output_dir()
{
  return __output_stats_dir;
}


static struct eztrace_counter* find_counter(eztrace_counter_id counter_id,
					    p_eztrace_container p_cont) {
  struct eztrace_counter* counter_array = root_counters;
  if(p_cont)
    counter_array = p_cont->counters;

  int i;
  for(i=0; i<nb_counters; i++) {
    if(counter_array[i].id == counter_id)
      return &counter_array[i];
  }
  return NULL;
}

/* Create a new counter whose id is counter_id. This counter contains vectors that consist in
 * vector_size items. The names of each of the vector item are stores in names
 */
void attach_counter(eztrace_counter_id counter_id, unsigned vector_size, char**names) {
  if(nb_counters >= MAX_COUNTERS) {
    fprintf(stderr, "Too many counters\n");
    exit(1);
  }
  struct eztrace_counter *cur_counter = &root_counters[nb_counters];

  /* check wether counter_id isn't already used */
  if(find_counter(counter_id, NULL)) {
    fprintf(stderr, "Error: Counter %d is already used!\n", counter_id);
    abort();
  }

  /* initialize the counter structure */
  cur_counter->id = counter_id;
  cur_counter->nb_values = vector_size;
  cur_counter->values = malloc(sizeof(double) * vector_size);
  cur_counter->value_names = malloc(sizeof(char*) * vector_size);

  int i;
  /* copy the values names */
  for(i=0; i<vector_size; i++) {
    asprintf(&(cur_counter->value_names[i]), "%s", names[i]);
    cur_counter->values[i] = 0;
  }
  asprintf(&(cur_counter->name), "Total");
  nb_counters++;
}

/* Return a pointer to the vector item #index from the counter #counter_id of a specific container */
double* get_counter(p_eztrace_container container, eztrace_counter_id counter_id, unsigned index) {
  struct eztrace_counter* cur_counter = find_counter(counter_id, container);
  if(cur_counter)
    return &(cur_counter->values[index]);
  return NULL;
}

/* Allocate counters in a newly created container */
void counters_new_container(p_eztrace_container p_cont) {
  assert(p_cont);

  p_cont->counters = malloc(sizeof(struct eztrace_counter)*nb_counters);
  int i;
  for(i=0;i<nb_counters;i++) {
    p_cont->counters[i].id = root_counters[i].id;
    p_cont->counters[i].name = root_counters[i].name;
    p_cont->counters[i].nb_values = root_counters[i].nb_values;
    p_cont->counters[i].values = malloc(sizeof(double)*p_cont->counters[i].nb_values);
    p_cont->counters[i].value_names = malloc(sizeof(char*)*p_cont->counters[i].nb_values);
    int j;
    for(j=0;j<p_cont->counters[i].nb_values; j++) {
      p_cont->counters[i].values[j] = 0;
      /* don't copy the value name. So be carefull where freeing it! */
      p_cont->counters[i].value_names[j] = root_counters[i].value_names[j];
    }
  }
}

static void reduce_counters(p_eztrace_container p_cont, eztrace_counter_id counter_id) {
  if(!p_cont)
    return;

  struct eztrace_counter* counter = find_counter(counter_id, p_cont);
  assert(counter);

  /* for each sub container, reduce its counters and append them to the local counter */
  int i, j;
  for(i=0; i<p_cont->nb_children; i++) {
    reduce_counters(p_cont->children[i], counter_id);

    struct eztrace_counter* children_counter = find_counter(counter_id, p_cont->children[i]);
    assert(children_counter);

    for(j=0; j<counter->nb_values; j++) {
      counter->values[j] += children_counter->values[j];
    }
  }
}



static void __print_containers_recurse(int depth, eztrace_counter_id id, p_eztrace_container p_cont) {
  if(!p_cont)
    return;
  struct eztrace_counter *counter = find_counter(id, p_cont);
  assert(counter);
  /* We assume that the reduction already happened */


  int i;
  double sum_values = 0;
  /* compute and print the sum of values */
  for(i=0; i<counter->nb_values; i++) {
    sum_values += counter->values[i];
  }
  for(i=0;i<depth; i++)
    printf("   ");
  printf("%s -- \t%s: %lf\t", p_cont->name, counter->name, sum_values);
  /* print each value */
  for(i=0; i<counter->nb_values; i++) {
    printf("%s: %lf\t", counter->value_names[i], counter->values[i]);
  }
  printf("\n");
  /* call the container children so that they print their values */
  for(i=0; i<p_cont->nb_children; i++) {
    __print_containers_recurse(depth+1,id, p_cont->children[i]);
  }
}



static void __print_counter(struct eztrace_counter* counter) {
  int i, j;
  /* todo: print the total of counters */
  for(i=0;i<NB_TRACES; i++) {
    /* reduce each trace counter */
    p_eztrace_container p_cont = GET_PROCESS_CONTAINER(i);
    reduce_counters(p_cont, counter->id);

    struct eztrace_counter* children_counter = find_counter(counter->id, p_cont);
    assert(children_counter);

    for(j=0; j<children_counter->nb_values; j++) {
      counter->values[j] += children_counter->values[j];
    }

    __print_containers_recurse(0, counter->id, p_cont);
  }
}

/* Print the values of a specific counter */
void print_counter(eztrace_counter_id id) {
  int i;

  for(i=0; i<nb_counters; i++) {
    if(root_counters[i].id == id)
      __print_counter(&root_counters[i]);
  }
}
