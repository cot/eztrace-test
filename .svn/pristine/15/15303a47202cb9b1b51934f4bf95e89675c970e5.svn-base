/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_HIERARCHICAL_ARRAY_H
#define EZTRACE_HIERARCHICAL_ARRAY_H
#include <assert.h>
#include "eztrace_convert.h"
#include "eztrace_stats_core.h"

struct hierarchical_array;

struct hierarchical_array_data {
  struct hierarchical_array *p_array;
  struct hierarchical_array_data *next;
  unsigned nb_allocated;
  unsigned nb_items;
  void* values;
};

struct hierarchical_array{
  eztrace_counter_id id;
  unsigned  item_size;
  unsigned nb_items;
  unsigned nb_allocated;
  struct hierarchical_array_data *first;
  struct hierarchical_array_data *last;
};

/* return the ith item of a hierarchical array */
static inline void* ITH_ITEM(int index, struct hierarchical_array* p_array)
{
  unsigned i = index;
  /* todo: start from the end ? */
  struct hierarchical_array_data *p_data = p_array->first;
  while(p_data) {
    if(i >= p_data->nb_allocated) {
      /* not in this array, go to the next one */
      i = i - p_data->nb_allocated;
      p_data = p_data->next;
    } else {
      return (void*)(p_data->values+(i*p_array->item_size));
    }
  }
  assert(1);
  return NULL;
}

/* Create a new counter whose id is counter_id. This counter contains vectors that consist in
 * vector_size items. The names of each of the vector item are stores in names
 */
void hierarchical_array_attach(eztrace_counter_id counter_id, unsigned item_size);

/* Allocate counters in a newly created container */
void hierarchical_array_new_container(p_eztrace_container p_cont);

/* return the number of items in the array (for a specific container) */
unsigned hierarchical_array_size(p_eztrace_container p_cont, eztrace_counter_id counter_id);

/* return the ith items of an array (for a specific container) */
void* hierarchical_array_get_item(p_eztrace_container p_cont, eztrace_counter_id counter_id, unsigned index);

/* return a new item of an array (for a specific container) */
void* hierarchical_array_new_item(p_eztrace_container p_cont, eztrace_counter_id counter_id);

struct hierarchical_array* hierarchical_array_find(eztrace_counter_id counter_id,
						   p_eztrace_container p_cont);

#endif	/* EZTRACE_HIERARCHICAL_ARRAY_H */
