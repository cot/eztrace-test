/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include "eztrace_stats_core.h"
#include "eztrace_hierarchical_array.h"

#define MAX_ARRAYS 16
#define DEFAULT_ALLOC_NB 128

/* todo: make this dynamic */
static struct hierarchical_array root_arrays[MAX_ARRAYS];
static unsigned nb_arrays = 0;


struct hierarchical_array* hierarchical_array_find(eztrace_counter_id counter_id,
						   p_eztrace_container p_cont)
{
  struct hierarchical_array* arrays = root_arrays;
  if(p_cont)
    arrays = p_cont->arrays;

  int i;
  for(i=0; i<nb_arrays; i++) {
    if(arrays[i].id == counter_id)
      return & arrays[i];
  }
  return NULL;
}

static void __hierarchical_array_expand(struct hierarchical_array* p_array,
					unsigned nb_item_to_alloc)
{
  p_array->nb_allocated += nb_item_to_alloc;
  /* initialize the new array */
  struct hierarchical_array_data* new_array = malloc(sizeof(struct hierarchical_array_data));
  new_array->p_array = p_array;
  new_array->next = NULL;
  new_array->nb_allocated = nb_item_to_alloc;
  new_array->nb_items = 0;
  new_array->values = malloc(p_array->item_size * nb_item_to_alloc);

  /* insert the new array at the end of p_array */
  if(p_array->last) {
    p_array->last->next = new_array;
    p_array->last = p_array->last->next;
  } else {
    /* the array is empty */
    p_array->first = new_array;
    p_array->last = new_array;
  }
}

static void __hierarchical_array_init(struct hierarchical_array* p_array,
				      eztrace_counter_id id,
				      unsigned item_size)
{
  assert(p_array);
  p_array->id = id;
  p_array->item_size = item_size;
  p_array->nb_items = 0;
  p_array->nb_allocated = 0;
  p_array->first = NULL;
  p_array->last = NULL;
  __hierarchical_array_expand(p_array, DEFAULT_ALLOC_NB);
}

/* Create a new counter whose id is counter_id. This counter contains vectors that consist in
 * vector_size items. The names of each of the vector item are stores in names
 */
void hierarchical_array_attach(eztrace_counter_id counter_id, unsigned item_size)
{
  if(nb_arrays >= MAX_ARRAYS) {
    fprintf(stderr, "Too many arrays\n");
    exit(1);
  }
  struct hierarchical_array *cur_array = &root_arrays[nb_arrays];

  /* check wether counter_id isn't already used */
  if(hierarchical_array_find(counter_id, NULL)) {
    fprintf(stderr, "Error: Counter %d is already used!\n", counter_id);
    abort();
  }

  /* initialize the array structure */
  __hierarchical_array_init(cur_array, counter_id, item_size);
  nb_arrays++;
}


/* Allocate counters in a newly created container */
void hierarchical_array_new_container(p_eztrace_container p_cont)
{
  assert(p_cont);

  p_cont->arrays = malloc(sizeof(struct hierarchical_array)*nb_arrays);
  int i;
  for(i=0;i<nb_arrays;i++) {
    __hierarchical_array_init(&(p_cont->arrays[i]),
			      root_arrays[i].id,
			      root_arrays[i].item_size);
  }
}

/* return the number of items in the array (for a specific container) */
unsigned hierarchical_array_size(p_eztrace_container p_cont, eztrace_counter_id counter_id)
{
  struct hierarchical_array* array = hierarchical_array_find(counter_id, p_cont);
  assert(array);
  return array->nb_items;
}

/* return the ith items of an array (for a specific container) */
void* hierarchical_array_get_item(p_eztrace_container p_cont, eztrace_counter_id counter_id, unsigned index)
{
  struct hierarchical_array* array = hierarchical_array_find(counter_id, p_cont);
  assert(array);
  assert(array->nb_items > index);
  return ITH_ITEM(index, array);
}

/* return a new item of an array (for a specific container) */
void* hierarchical_array_new_item(p_eztrace_container p_cont, eztrace_counter_id counter_id)
{
  struct hierarchical_array* array = hierarchical_array_find(counter_id, p_cont);
  assert(array);

  if(array->nb_items >= array->nb_allocated) {
    /* no space left in the array, expand it */
    __hierarchical_array_expand(array, array->nb_allocated);
    assert(array->nb_items < array->nb_allocated);
  }

  void* res = ITH_ITEM(array->nb_items, array);
  array->nb_items++;
  return res;
}

