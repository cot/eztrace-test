
#ifndef __staticlib_EV_CODES_H__
#define __staticlib_EV_CODES_H__

/* This file defines the event codes that are used by the example
 * module.
 */
#include "ev_codes.h"

/* Event codes prefix. This identifies the module and thus should be
 * unique.
 * The 0x0? prefix is reserved for eztrace internal use. Thus you can
 * use any prefix between 0x10 and 0xff.
 */
#define staticlib_EVENTS_ID    0x99
#define staticlib_PREFIX       (staticlib_EVENTS_ID << NB_BITS_EVENTS)

/* Define various event codes used by the example module
 * The 2 most significant bytes should correspond to the module id,
 * as below:
 */
#define FUT_staticlib_mafunc_1 (staticlib_PREFIX | 0x1)
#define FUT_staticlib_mafunc_2 (staticlib_PREFIX | 0x2)


#endif	/* __staticlib_EV_CODES_H__ */
