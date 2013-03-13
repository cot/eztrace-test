/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef __EXAMPLE_EV_CODES_H__
#define __EXAMPLE_EV_CODES_H__

/* This file defines the event codes that are used by the example
 * module.
 */
#include "ev_codes.h"

/* Event codes prefix. This identifies the module and thus should be
 * unique.
 * The 0x0? prefix is reserved for eztrace internal use. Thus you can
 * use any prefix between 0x10 and 0xff.
 */
#define EXAMPLE_EVENTS_ID    0x11
#define EXAMPLE_PREFIX       (EXAMPLE_EVENTS_ID << NB_BITS_EVENTS)

/* Define various event codes used by the example module
 * The 2 most significant bytes should correspond to the module id,
 * as below:
 */
#define FUT_EXAMPLE_FUNCTION1_ENTRY  (EXAMPLE_PREFIX | 0x0001)
#define FUT_EXAMPLE_FUNCTION1_EXIT   (EXAMPLE_PREFIX | 0x0002)

#define FUT_EXAMPLE_FUNCTION2_ENTRY  (EXAMPLE_PREFIX | 0x0011)
#define FUT_EXAMPLE_FUNCTION2_EXIT   (EXAMPLE_PREFIX | 0x0012)

#define FUT_STATIC_EXAMPLE_FUNCTION_ENTRY  (EXAMPLE_PREFIX | 0x0101)
#define FUT_STATIC_EXAMPLE_FUNCTION_EXIT   (EXAMPLE_PREFIX | 0x0102)

#endif	/* __EXAMPLE_EV_CODES_H__ */
