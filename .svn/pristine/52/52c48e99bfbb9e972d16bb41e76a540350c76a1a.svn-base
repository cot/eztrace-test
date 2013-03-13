/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef __PAPI_EV_CODES_H__
#define __PAPI_EV_CODES_H__

#include <papi.h>
#include "ev_codes.h"


#define PAPI_EVENTS_ID 0x06
#define PAPI_PREFIX    (PAPI_EVENTS_ID << NB_BITS_EVENTS)

#define FUT_PAPI_INIT            (PAPI_PREFIX | 0X0001)
#define FUT_PAPI_INIT_COUNTERS   (PAPI_PREFIX | 0X0002)
#define FUT_PAPI_MEASUREMENT     (PAPI_PREFIX | 0X0010)


struct papi_counter_id {
  int code;
  char* code_str;
  char* description;
};

struct papi_counter_id *papi_counter_ids;
static int nb_counters = 0;

static inline void __papi_print_counter_ids() {
  int i;
  for(i=0; i<nb_counters; i++) {
    printf("[%d] code %x : %s (%s)\n", i, papi_counter_ids[i].code,
	   papi_counter_ids[i].code_str, papi_counter_ids[i].description);
  }
}

static inline void __papi_print_available_counters() {
  PAPI_event_info_t info;
  int retval;
  int mask = PAPI_PRESET_ENUM_AVAIL;
  int start = 0 | PAPI_PRESET_MASK;
  int i = start;

  do {
    retval = PAPI_get_event_info(i, &info);
    if (retval == PAPI_OK) {
      printf("%-20s %s\n", info.symbol, info.long_descr);
    }
  }  while ((retval = PAPI_enum_event(&i, mask  )) == PAPI_OK);
}

/* search for a counter name and return the corresponding code.
 * if not found, return -1;
 */
static inline int __papi_get_counter_id(char* counter_name) {
  int i;
  for(i=0; i<nb_counters; i++) {
    if(strcmp(counter_name, papi_counter_ids[i].code_str) == 0){
      return i;
    }
  }
  return -1;
}

static inline int __papi_get_counter_id_by_code(int counter_code) {
  int i;
  for(i=0; i<nb_counters; i++) {
    if(counter_code == papi_counter_ids[i].code)
      return i;
  }
  return -1;
}

#define define_counter(id, __code__, __code_str__, __desc__)		\
  {									\
    papi_counter_ids[id].code = __code__;				\
    asprintf(&(papi_counter_ids[id].code_str), "%s", __code_str__);	\
    asprintf(&(papi_counter_ids[id].description), "%s", __desc__);	\
  }

static inline void __papi_init_counter_ids() {
  nb_counters = 0;
  papi_counter_ids = NULL;

  PAPI_event_info_t info;
  int retval;
  int cur_counter = 0;
  int mask = PAPI_PRESET_ENUM_AVAIL;
  int start = 0 | PAPI_PRESET_MASK;
  int i = start;

  do {
    retval = PAPI_get_event_info(i, &info);
    if (retval == PAPI_OK) nb_counters ++;
  }  while ((retval = PAPI_enum_event(&i, mask  )) == PAPI_OK);
  papi_counter_ids = malloc(sizeof(struct papi_counter_id) *  nb_counters);
  i = start;

  do {
    retval = PAPI_get_event_info(i, &info);
    if (retval == PAPI_OK) {
      define_counter(cur_counter, info.event_code, info.symbol, info.long_descr);
      cur_counter ++;
    }
  }  while ((retval = PAPI_enum_event(&i, mask  )) == PAPI_OK);
}

#endif	/* _PAPI_EV_CODES_H__ */
