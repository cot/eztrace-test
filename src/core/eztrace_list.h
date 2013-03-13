/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef EZTRACE_LIST_H
#define EZTRACE_LIST_H

#include <stdlib.h>

struct ezt_list_t;

struct ezt_list_token_t {
  /* pointer to the user data */
  void *data;
  /* pointer to the list */
  struct ezt_list_t *list;
  /* pointer to the next token in the list */
  struct ezt_list_token_t* next;
  /* pointer to the previous token in the list */
  struct ezt_list_token_t* prev;
};

struct ezt_list_t {
  struct ezt_list_token_t *head;
  struct ezt_list_token_t *tail;
  int nb_item;
};

/* return true if the list is empty */
#define ezt_list_empty(l) ((l)->head == NULL)

/* iterate over the list */
#define ezt_list_foreach(l, t) for(t = (l)->head; t != NULL; t = (t)->next)

/* same as list_foreach, but supports list modifications */
#define ezt_list_foreach_safe(l, t, n) for(t = (l)->head, (n) = ((l)->head?t->next:NULL); \
					   n != NULL;			\
					   t = n, (n) = t->next)

/* return the first token of the list */
#define ezt_list_get_head(l) ((l)->head)

/* return the last token of the list */
#define ezt_list_get_tail(l) ((l)->tail)


/* initialize a list */
static inline void ezt_list_new(struct ezt_list_t*l)
{
  l->head = NULL;
  l->tail = NULL;
  l->nb_item = 0;
}

/* add a new token at the list tail */
static inline void ezt_list_add(struct ezt_list_t*l, struct ezt_list_token_t *n)
{
  /* initialize the new token */
  n->list = l;
  n->next = NULL;
  n->prev = l->tail;

  if(ezt_list_empty(l)) {
    l->head = n;
  } else {
    l->tail->next = n;
  }
  l->tail = n;
  l->nb_item ++;
}

/* remove a token from the list */
static inline void ezt_list_remove(struct ezt_list_token_t *t)
{
  if(t->prev)
    t->prev->next = t->next;
  else
    t->list->head = t->next;

  if(t->next)
    t->next->prev = t->prev;
  else
    t->list->tail = t->prev;

  if(t->list->head && !t->list->tail)
    *(int*)0=0;
  t->list->nb_item --;
}

#endif	/* EZTRACE_LIST_H */
