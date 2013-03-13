/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_ARRAY_H
#define EZTRACE_ARRAY_H

#include <stdint.h>

struct eztrace_array_t {
  unsigned  item_size;
  unsigned nb_items;
  void* values;
  unsigned nb_allocated;
};

/* return the ith value of an array */
#define ITH_VALUE(__i__, __array__) (void*)(((__array__)->values) + ((__i__)*(__array__)->item_size))

/* initialize an array */
void eztrace_array_create(struct eztrace_array_t* p_array,
			  unsigned item_size,
			  unsigned nb_prealloc);

/* free an array */
void eztrace_array_free(struct eztrace_array_t* p_array);

/* return a new item of an array (for a specific container) */
void* eztrace_array_new_value(struct eztrace_array_t* p_array);

#endif	/* EZTRACE_ARRAY_H */
