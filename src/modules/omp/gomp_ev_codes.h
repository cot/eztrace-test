/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef __GOMP_EV_CODES_H__
#define __GOMP_EV_CODES_H__

#include "ev_codes.h"

#define GOMP_EVENTS_ID    0x01
#define GOMP_PREFIX         (GOMP_EVENTS_ID << NB_BITS_EVENTS)

#define FUT_GOMP_PARALLEL_START (GOMP_PREFIX | 0x0001)
#define FUT_GOMP_NEW_FORK       (GOMP_PREFIX | 0x0002)
#define FUT_GOMP_NEW_JOIN       (GOMP_PREFIX | 0x0003)
#define FUT_GOMP_JOIN_DONE      (GOMP_PREFIX | 0x0004)

#define FUT_GOMP_CRITICAL_START       (GOMP_PREFIX | 0x0005)
#define FUT_GOMP_CRITICAL_START_DONE  (GOMP_PREFIX | 0x0006)
#define FUT_GOMP_CRITICAL_STOP        (GOMP_PREFIX | 0x0007)

#define FUT_POMP2_FINALIZE        (GOMP_PREFIX | 0xffff)

#define FUT_POMP2_ATOMIC_ENTER    (GOMP_PREFIX | 0x0010)
#define FUT_POMP2_ATOMIC_EXIT     (GOMP_PREFIX | 0x0011)

#define FUT_POMP2_BARRIER_ENTER   (GOMP_PREFIX | 0x0020)
#define FUT_POMP2_BARRIER_EXIT    (GOMP_PREFIX | 0x0021)

#define FUT_POMP2_FLUSH_ENTER     (GOMP_PREFIX | 0x0030)
#define FUT_POMP2_FLUSH_EXIT      (GOMP_PREFIX | 0x0031)

#define FUT_POMP2_CRITICAL_ENTER  (GOMP_PREFIX | 0x0040)
#define FUT_POMP2_CRITICAL_EXIT   (GOMP_PREFIX | 0x0041)
#define FUT_POMP2_CRITICAL_BEGIN  (GOMP_PREFIX | 0x0042)
#define FUT_POMP2_CRITICAL_END    (GOMP_PREFIX | 0x0043)

#define FUT_POMP2_FOR_ENTER       (GOMP_PREFIX | 0x0050)
#define FUT_POMP2_FOR_EXIT        (GOMP_PREFIX | 0x0051)

#define FUT_POMP2_MASTER_BEGIN    (GOMP_PREFIX | 0x0060)
#define FUT_POMP2_MASTER_END      (GOMP_PREFIX | 0x0061)

#define FUT_POMP2_PARALLEL_BEGIN  (GOMP_PREFIX | 0x0070)
#define FUT_POMP2_PARALLEL_END    (GOMP_PREFIX | 0x0071)
#define FUT_POMP2_PARALLEL_FORK   (GOMP_PREFIX | 0x0072)
#define FUT_POMP2_PARALLEL_JOIN   (GOMP_PREFIX | 0x0073)

#define FUT_POMP2_SECTIONS_ENTER  (GOMP_PREFIX | 0x0080)
#define FUT_POMP2_SECTIONS_EXIT   (GOMP_PREFIX | 0x0081)

#define FUT_POMP2_SECTION_BEGIN   (GOMP_PREFIX | 0x0082)
#define FUT_POMP2_SECTION_END    (GOMP_PREFIX | 0x0083)

#define FUT_POMP2_SINGLE_ENTER    (GOMP_PREFIX | 0x0090)
#define FUT_POMP2_SINGLE_EXIT     (GOMP_PREFIX | 0x0091)
#define FUT_POMP2_SINGLE_BEGIN    (GOMP_PREFIX | 0x0092)
#define FUT_POMP2_SINGLE_END     (GOMP_PREFIX | 0x0093)

#define FUT_POMP2_WORKSHARE_ENTER (GOMP_PREFIX | 0x00a0)
#define FUT_POMP2_WORKSHARE_EXIT  (GOMP_PREFIX | 0x00a1)

#define FUT_POMP2_TASK_BEGIN        (GOMP_PREFIX | 0x00b0)
#define FUT_POMP2_TASK_END          (GOMP_PREFIX | 0x00b1)
#define FUT_POMP2_TASK_CREATE_BEGIN (GOMP_PREFIX | 0x00b2)
#define FUT_POMP2_TASK_CREATE_END   (GOMP_PREFIX | 0x00b3)

#define FUT_POMP2_UNTIED_TASK_BEGIN        (GOMP_PREFIX | 0x00b4)
#define FUT_POMP2_UNTIED_TASK_END          (GOMP_PREFIX | 0x00b5)
#define FUT_POMP2_UNTIED_TASK_CREATE_BEGIN (GOMP_PREFIX | 0x00b6)
#define FUT_POMP2_UNTIED_TASK_CREATE_END   (GOMP_PREFIX | 0x00b7)

#define FUT_POMP2_TASKWAIT_BEGIN    (GOMP_PREFIX | 0x00b8)
#define FUT_POMP2_TASKWAIT_END      (GOMP_PREFIX | 0x00b9)

#define FUT_POMP2_TASK_INFO         (GOMP_PREFIX | 0x00ba)

#ifdef OPENMP_FOUND
#define FUT_POMP2_SET_LOCK_ENTRY    (GOMP_PREFIX | 0x0101)
#define FUT_POMP2_SET_LOCK_EXIT     (GOMP_PREFIX | 0x0102)
#define FUT_POMP2_TEST_LOCK_SUCCESS (GOMP_PREFIX | 0x0103)
#define FUT_POMP2_UNSET_LOCK        (GOMP_PREFIX | 0x0104)

#define FUT_POMP2_SET_NEST_LOCK_ENTRY    (GOMP_PREFIX | 0x0111)
#define FUT_POMP2_SET_NEST_LOCK_EXIT     (GOMP_PREFIX | 0x0112)
#define FUT_POMP2_TEST_NEST_LOCK_SUCCESS (GOMP_PREFIX | 0x0113)
#define FUT_POMP2_UNSET_NEST_LOCK        (GOMP_PREFIX | 0x0114)
#endif /* OPENMP_FOUND */

#endif	/* __GOMP_EV_CODES_H__ */
