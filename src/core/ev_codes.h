/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef __EV_CODES_H__
#define __EV_CODES_H__

#define NB_BITS 24
#define NB_BITS_PREFIX 8
#define NB_BITS_EVENTS (NB_BITS - NB_BITS_PREFIX)

#define PTHREAD_CORE_EVENTS_ID 0x00
#define PTHREAD_CORE_PREFIX    (PTHREAD_CORE_EVENTS_ID << NB_BITS_EVENTS)

/* thread creation/destruction */
#define FUT_NEW_THREAD          (PTHREAD_CORE_PREFIX | 0x0001)
#define FUT_END_THREAD          (PTHREAD_CORE_PREFIX | 0x0002)
#define FUT_START_THREAD_JOIN   (PTHREAD_CORE_PREFIX | 0x0004)
#define FUT_STOP_THREAD_JOIN    (PTHREAD_CORE_PREFIX | 0x0005)

#define FUT_SIGNAL_RECEIVED     (PTHREAD_CORE_PREFIX | 0xf000)
#define FUT_CALLING_FUNCTION    (PTHREAD_CORE_PREFIX | 0xf001)

#endif	/*  __EV_CODES_H__ */
