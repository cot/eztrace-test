/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef __MPI_EV_CODES_H__
#define __MPI_EV_CODES_H__

#include "ev_codes.h"

#define MPI_EVENTS_ID     0x04
#define MPI_PREFIX        (MPI_EVENTS_ID << NB_BITS_EVENTS)


#define FUT_MPI_INIT           (MPI_PREFIX | 0x0010)
#define FUT_MPI_INIT_Info      (MPI_PREFIX | 0x0011)

#define FUT_MPI_NEW_COMM       (MPI_PREFIX | 0x0012)
#define FUT_MPI_NEW_COMM_Info  (MPI_PREFIX | 0x0013)

#define FUT_MPI_START_SEND     (MPI_PREFIX | 0x0001)
#define FUT_MPI_STOP_SEND      (MPI_PREFIX | 0x0002)
#define FUT_MPI_START_BSEND    (MPI_PREFIX | 0x0003)
#define FUT_MPI_STOP_BSEND     (MPI_PREFIX | 0x0004)
#define FUT_MPI_START_SSEND    (MPI_PREFIX | 0x0005)
#define FUT_MPI_STOP_SSEND     (MPI_PREFIX | 0x0006)
#define FUT_MPI_START_RSEND    (MPI_PREFIX | 0x0007)
#define FUT_MPI_STOP_RSEND     (MPI_PREFIX | 0x0008)

#define FUT_MPI_START_SENDRECV         (MPI_PREFIX | 0x000a)
#define FUT_MPI_STOP_SENDRECV          (MPI_PREFIX | 0x000b)
#define FUT_MPI_START_SENDRECV_REPLACE (MPI_PREFIX | 0x000c)
#define FUT_MPI_STOP_SENDRECV_REPLACE  (MPI_PREFIX | 0x000d)

#define FUT_MPI_ISEND    (MPI_PREFIX | 0x0101)
#define FUT_MPI_IBSEND   (MPI_PREFIX | 0x0103)
#define FUT_MPI_ISSEND   (MPI_PREFIX | 0x0105)
#define FUT_MPI_IRSEND   (MPI_PREFIX | 0x0107)
#define FUT_MPI_START_PUT   (MPI_PREFIX | 0x0201)
#define FUT_MPI_STOP_PUT    (MPI_PREFIX | 0x0202)
#define FUT_MPI_START_GET   (MPI_PREFIX | 0x0203)
#define FUT_MPI_STOP_GET    (MPI_PREFIX | 0x0204)

#define FUT_MPI_START_RECV     (MPI_PREFIX | 0x1003)
#define FUT_MPI_STOP_RECV      (MPI_PREFIX | 0x1004)
#define FUT_MPI_IRECV          (MPI_PREFIX | 0x1005)

#define FUT_MPI_START_WAIT     (MPI_PREFIX | 0x2001)
#define FUT_MPI_STOP_WAIT      (MPI_PREFIX | 0x2002)
#define FUT_MPI_TEST_SUCCESS   (MPI_PREFIX | 0x2003)
#define FUT_MPI_START_PROBE    (MPI_PREFIX | 0x2004)
#define FUT_MPI_STOP_PROBE     (MPI_PREFIX | 0x2005)
#define FUT_MPI_IPROBE_SUCCESS (MPI_PREFIX | 0x2006)
#define FUT_MPI_IPROBE_FAILED  (MPI_PREFIX | 0x2007)
#define FUT_MPI_START_WAITANY     (MPI_PREFIX | 0x2008)
#define FUT_MPI_STOP_WAITANY      (MPI_PREFIX | 0x2009)
#define FUT_MPI_START_WAITALL     (MPI_PREFIX | 0x200a)
#define FUT_MPI_STOP_WAITALL      (MPI_PREFIX | 0x200b)

#define FUT_MPI_BCast_info           (MPI_PREFIX | 0x3101)

#define FUT_MPI_START_BCast          (MPI_PREFIX | 0x3001)
#define FUT_MPI_START_Gather         (MPI_PREFIX | 0x3002)
#define FUT_MPI_START_Gatherv        (MPI_PREFIX | 0x3003)
#define FUT_MPI_START_Scatter        (MPI_PREFIX | 0x3004)
#define FUT_MPI_START_Scatterv       (MPI_PREFIX | 0x3005)
#define FUT_MPI_START_Allgather      (MPI_PREFIX | 0x3006)
#define FUT_MPI_START_Allgatherv     (MPI_PREFIX | 0x3007)
#define FUT_MPI_START_Alltoall       (MPI_PREFIX | 0x3008)
#define FUT_MPI_START_Alltoallv      (MPI_PREFIX | 0x3009)
#define FUT_MPI_START_Reduce         (MPI_PREFIX | 0x300a)
#define FUT_MPI_START_Allreduce      (MPI_PREFIX | 0x300b)
#define FUT_MPI_START_Reduce_scatter (MPI_PREFIX | 0x300c)
#define FUT_MPI_START_Scan           (MPI_PREFIX | 0x300d)
#define FUT_MPI_START_BARRIER        (MPI_PREFIX | 0x300e)

#define FUT_MPI_STOP_BCast           (MPI_PREFIX | 0x3011)
#define FUT_MPI_STOP_Gather          (MPI_PREFIX | 0x3012)
#define FUT_MPI_STOP_Gatherv         (MPI_PREFIX | 0x3013)
#define FUT_MPI_STOP_Scatter         (MPI_PREFIX | 0x3014)
#define FUT_MPI_STOP_Scatterv        (MPI_PREFIX | 0x3015)
#define FUT_MPI_STOP_Allgather       (MPI_PREFIX | 0x3016)
#define FUT_MPI_STOP_Allgatherv      (MPI_PREFIX | 0x3017)
#define FUT_MPI_STOP_Alltoall        (MPI_PREFIX | 0x3018)
#define FUT_MPI_STOP_Alltoallv       (MPI_PREFIX | 0x3019)
#define FUT_MPI_STOP_Reduce          (MPI_PREFIX | 0x301a)
#define FUT_MPI_STOP_Allreduce       (MPI_PREFIX | 0x301b)
#define FUT_MPI_STOP_Reduce_scatter  (MPI_PREFIX | 0x301c)
#define FUT_MPI_STOP_Scan            (MPI_PREFIX | 0x301d)
#define FUT_MPI_STOP_BARRIER         (MPI_PREFIX | 0x301e)

#define FUT_MPI_SPAWN                (MPI_PREFIX | 0x4001)
#define FUT_MPI_SPAWNED              (MPI_PREFIX | 0x4002)

#define FUT_MPI_SEND_INIT            (MPI_PREFIX | 0x5001)
#define FUT_MPI_BSEND_INIT           (MPI_PREFIX | 0x5002)
#define FUT_MPI_RSEND_INIT           (MPI_PREFIX | 0x5003)
#define FUT_MPI_SSEND_INIT           (MPI_PREFIX | 0x5004)
#define FUT_MPI_RECV_INIT            (MPI_PREFIX | 0x5010)
#define FUT_MPI_START                (MPI_PREFIX | 0x5100)

#define FUT_MPI_Info                 (MPI_PREFIX | 0x9999)


#endif	/* __MPI_EV_CODES_H__ */

