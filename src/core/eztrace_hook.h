/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */
#ifndef EZTRACE_HOOK_H
#define EZTRACE_HOOK_H

#include <stdint.h>
#include "eztrace.h"
#include "eztrace_list.h"

/* 
 * This file defines a hook mechanism that allow different modules
 * (ie. eztrace_convert_mpi, eztrace_convert_gomp, etc.) to add
 * information in EZTrace internal structures. This allows to add
 * specialized information (such as the current parallel section in
 * case OpenMP is being used) to any general purpose structure.
 */

struct ezt_hook_list_t;

typedef uint8_t ezt_hook_id_t;

struct ezt_hook_t {
  ezt_hook_id_t hook_id;
  void* data;
  struct ezt_hook_list_t* p_hook_list;  
  struct ezt_list_token_t list_token;
};

struct ezt_hook_list_t {
  struct ezt_list_t hook_list;
};

/* initialize a hook list */
static inline void ezt_hook_list_init(struct ezt_hook_list_t *p_list);
/* free the hook structure. before calling this function, make sure all
 * the data pointers have been freed.
 */
static inline void ezt_hook_list_free(struct ezt_hook_list_t *p_list);
/* add a new hook to the list. */
static inline void ezt_hook_list_add(struct ezt_hook_list_t *p_list, void*p_data, ezt_hook_id_t hook_id);
/* remove a hook from the list. Return 0 if the hook cannot be found in the list */
static inline int ezt_hook_list_remove(struct ezt_hook_list_t *p_list, ezt_hook_id_t hook_id);
/* retrive the data corresponding to a hook id or NULL if the hook is not in the list */
static inline void* ezt_hook_list_retrieve_data(struct ezt_hook_list_t *p_hook_list, ezt_hook_id_t hook_id);


/* initialize a hook list */
static inline void ezt_hook_list_init(struct ezt_hook_list_t *p_list)
{
  ezt_list_new(&p_list->hook_list);
}

/* free the hook structure. before calling this function, make sure all
 * the data pointers have been freed.
 */
static inline void ezt_hook_list_free(struct ezt_hook_list_t *p_list)
{
  struct ezt_list_token_t *p_token, *temp;
  ezt_list_foreach_safe(&p_list->hook_list, temp, p_token) {
    struct ezt_hook_t *p_hook =
      ezt_container_of(p_token, struct ezt_hook_t, list_token);
    ezt_list_remove(&p_hook->list_token);
    if(p_hook->data)
      free(p_hook->data);
    free(p_hook);
  }
}

/* add a new hook to the list. */
static inline void ezt_hook_list_add(struct ezt_hook_list_t *p_list, void*p_data, ezt_hook_id_t hook_id)
{

#if DEBUG
  if( ezt_hook_list_retrieve_data(p_list, hook_id)) {
    fprintf(stderr, "Trying to add a hook (id %d) in a list that already contains it\n", hook_id);
    abort();
  }
#endif	/* DEBUG */

  struct ezt_hook_t *p_hook = (struct ezt_hook_t*) malloc(sizeof(struct ezt_hook_t));
  p_hook->hook_id = hook_id;
  p_hook->p_hook_list = p_list;
  p_hook->data = p_data;
  ezt_list_add(&p_list->hook_list, &p_hook->list_token);
  return ;
}

/* remove a hook from the list. Return 0 if the hook cannot be found in the list */
static inline int ezt_hook_list_remove(struct ezt_hook_list_t *p_list, ezt_hook_id_t hook_id)
{
  struct ezt_list_token_t *p_token, *temp;
  ezt_list_foreach_safe(&p_list->hook_list, temp, p_token) {
    struct ezt_hook_t *p_hook = ezt_container_of(p_token, struct ezt_hook_t, list_token);
    if(p_hook->hook_id == hook_id) {
      ezt_list_remove(&p_hook->list_token);
      free(p_hook);
      return 1;
    }
  }
  return 0;
}

/* retrive the data corresponding to a hook id or NULL if the hook is not in the list */
static inline void* ezt_hook_list_retrieve_data(struct ezt_hook_list_t *p_list, ezt_hook_id_t hook_id)
{
  int i = 0;
  struct ezt_list_token_t *p_token;
  ezt_list_foreach(&p_list->hook_list, p_token) {
    i++;
    struct ezt_hook_t *p_hook = ezt_container_of(p_token, struct ezt_hook_t, list_token);
    if(p_hook->hook_id == hook_id)
      return p_hook->data;
  }
  return NULL;
}

#endif	/* EZTRACE_HOOK_H */
