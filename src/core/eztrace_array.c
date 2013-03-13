/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <assert.h>
#include <stdlib.h>
#include "eztrace_array.h"

#define DEFAULT_PREALLOC 64

/* initialize an array */
void eztrace_array_create(struct eztrace_array_t* p_array,
			  unsigned item_size,
			  unsigned nb_prealloc)
{
  p_array->item_size    = item_size;
  p_array->nb_items     = 0;
  p_array->values       = malloc(item_size * nb_prealloc);
  p_array->nb_allocated = nb_prealloc;
}

/* free an array */
void eztrace_array_free(struct eztrace_array_t* p_array)
{
  assert(p_array);
  free(p_array->values);
}

/* return a new item of an array (for a specific container) */
void* eztrace_array_new_value(struct eztrace_array_t* p_array)
{
  assert(p_array);

  if(p_array->nb_items >= p_array->nb_allocated) {
    /* expand the buffer */
    p_array->nb_allocated *= 2;
    if(! p_array->nb_allocated)
      p_array->nb_allocated = DEFAULT_PREALLOC;

    p_array->values = realloc(p_array->values, p_array->nb_allocated);
  }

  p_array->nb_items ++;
  return ITH_VALUE(p_array->nb_items - 1, p_array);
}
