/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef __MEMORY_EV_CODES_H__
#define __MEMORY_EV_CODES_H__

#include "ev_codes.h"


#define MEMORY_EVENTS_ID 0x05
#define MEMORY_PREFIX    (MEMORY_EVENTS_ID << NB_BITS_EVENTS)

#define FUT_MEMORY_MALLOC    (MEMORY_PREFIX | 0X0001)
#define FUT_MEMORY_CALLOC    (MEMORY_PREFIX | 0X0002)
#define FUT_MEMORY_REALLOC   (MEMORY_PREFIX | 0X0003)
#define FUT_MEMORY_FREE      (MEMORY_PREFIX | 0X0010)

#endif	/* __MEMORY_EV_CODES_H__ */
