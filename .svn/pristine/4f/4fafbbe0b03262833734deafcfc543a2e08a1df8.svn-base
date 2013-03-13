/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_STACK_H
#define EZTRACE_STACK_H

#include "eztrace_list.h"

typedef struct ezt_list_t       ezt_stack_t;
typedef struct ezt_list_token_t ezt_stack_token_t;

/* todo: a stack is just like a list, but with different functions */

#define ezt_stack_empty(s) (ezt_list_empty(l))

/* create an empty stack */
static inline void ezt_stack_new(ezt_stack_t *s)
{
  ezt_list_new(s);
}

/* add a new token to the stack */
static inline void ezt_stack_push(ezt_stack_t *s, ezt_stack_token_t *n)
{
  ezt_list_add(s, n);
}

/* return a pointer to the token on top of the stack */
static inline ezt_stack_token_t* ezt_stack_get_top(ezt_stack_t *s)
{
  return s->tail;
}

/* remove the token on top of the stack and return it.
 */
static inline ezt_stack_token_t* ezt_stack_pop(ezt_stack_t *s)
{
  if(!s->tail)
    return NULL;

  ezt_stack_token_t* res = s->tail;
  ezt_list_remove(res);
  return res;
}

#endif	/* EZTRACE_STACK_H */
