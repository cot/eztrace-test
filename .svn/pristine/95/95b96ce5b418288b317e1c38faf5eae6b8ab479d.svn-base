/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE 1 /* or _BSD_SOURCE or _SVID_SOURCE */
#define _REENTRANT

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <papi.h>

#include "papi_ev_codes.h"
#include "eztrace.h"

START_INTERCEPT END_INTERCEPT

/* set to 1 when all the hooks are set.
 * This is usefull in order to avoid recursive calls
 */
static int __papi_initialized = 0;

/* delay between 2 measurements (in us) */
#define TICK_THRESHOLD 1000

//#define VERBOSE 1
/* We don't use the FUNCTION_NAME macro (defined in src/core/eztrace.h)
 * to avoid infinite loops (since it calls record_counters).
 */
#ifdef FUNCTION_NAME
#undef FUNCTION_NAME
#endif

#ifdef VERBOSE
#define FUNCTION_NAME fprintf(stderr, "Calling [%s]\n", __FUNCTION__)
#else
#define FUNCTION_NAME (void) 0
#endif

#define TIME_DIFF(t1, t2) \
        ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

static pthread_key_t papi_thread_info_key;

struct __papi_thread_info {
    int *Events;
    long_long *values;
    struct timeval last_tick;
};

static int nb_events;
static int *Events;

struct __papi_thread_info * papi_init_hw_counter();

#define handle_error(n) \
  fprintf(stderr, "PAPI error %d: %s\n",n,PAPI_strerror(n))

void papi_record() {
    if (!__papi_initialized || !nb_events)
        return;

    struct __papi_thread_info *ptr = NULL;
    struct timeval cur_tick;
    gettimeofday(&cur_tick, NULL );

    /* retrieve the thread-specific structure */
    if ((ptr = pthread_getspecific(papi_thread_info_key)) == NULL ) {
        ptr = papi_init_hw_counter();
        if (!ptr)
            return;
    }

    /* did we record counters not so long ago ? */
    if (TIME_DIFF(ptr->last_tick, cur_tick) < TICK_THRESHOLD) {
        return;
    }

    /* read hardware counter */
    // this region needs to be protected in order to disable infinite recursion due to the intercepting of functions.
    // Because, PAPI functions can possibly be intercepted too.
    EZTRACE_PROTECT {
        EZTRACE_PROTECT_ON();
        int ret;
        if ((ret = PAPI_read_counters(ptr->values, nb_events)) != PAPI_OK) {
            fprintf(stderr, "error %d in PAPI_read_counters\n", ret);
            fprintf(stderr, "nb_events %d\n", nb_events);
            int i;
            for (i = 0; i < nb_events; i++) {
                fprintf(stderr, "values[%d] = %lld\n", i, ptr->values[i]);
            }

            handle_error(ret);
        }

        int duration = TIME_DIFF(ptr->last_tick, cur_tick); /* duration in usec */
        int i;
        for (i = 0; i < nb_events; i++)
            // As this region is protected, the unprotected function is called
            EZTRACE_EVENT3_UNPROTECTED(FUT_PAPI_MEASUREMENT, i, ptr->values[i], duration);

        /* reset the timer */
        ptr->last_tick.tv_usec = cur_tick.tv_usec;
        ptr->last_tick.tv_sec = cur_tick.tv_sec;

        EZTRACE_PROTECT_OFF();
    }
}

struct __papi_thread_info * papi_init_hw_counter() {

    if (!nb_events)
        return NULL ;

    struct __papi_thread_info *ptr = NULL;
    // this region needs to be protected in order to disable infinite recursion due to the intercepting of functions.
    // Because, PAPI functions can possibly be intercepted too.
    EZTRACE_PROTECT {
        EZTRACE_PROTECT_ON();

        int ret;
        /* initialize the thread-specific structure */
        if ((ptr = pthread_getspecific(papi_thread_info_key)) == NULL ) {

            ptr = malloc(sizeof(struct __papi_thread_info));
            ptr->Events = malloc(sizeof(int) * nb_events);
            ptr->values = malloc(sizeof(long_long) * nb_events);
            int i;
            for (i = 0; i < nb_events; i++) {
                ptr->Events[i] = Events[i];
            }

            if ((ret = PAPI_start_counters(ptr->Events, nb_events)) != PAPI_OK) {
                if (ret == PAPI_ECNFLCT) {
                    fprintf(stderr, "PAPI error %d (%s): It is likely that you selected too many counters to monitor\n",
                            ret, PAPI_strerror(ret));
                    exit(-1);
                }
            }

            (void) pthread_setspecific(papi_thread_info_key, ptr);

            /* first measurement */
            gettimeofday(&ptr->last_tick, NULL );
            if ((ret = PAPI_read_counters(ptr->values, nb_events)) != PAPI_OK)
                handle_error(ret);
        }
        EZTRACE_PROTECT_OFF();
    }
    return ptr;
}

/* select the counters to monitor */
static void __papi_select_counters() {
    int invalid_counters = 0;
    char* env1 = getenv("EZTRACE_PAPI_COUNTERS");
    if (env1) {
        /* EZTRACE_PAPI_COUNTERS is defined */
        nb_events = 0;
        Events = NULL;

        /* strtok may modify env, so let's make a copy of it */
        char* env;
        asprintf(&env, "%s", env1);
        char *token = strtok(env, " ");
        /* for each token, get the code id and fill the Events array */
        while (token) {
            nb_events++;
            Events = realloc(Events, sizeof(int) * nb_events);
            int id = __papi_get_counter_id(token);
            if (id == -1) {
                fprintf(stderr, "Unknown PAPI counter '%s'\n", token);
                invalid_counters = 1;
                nb_events--;
            } else {
                Events[nb_events - 1] = papi_counter_ids[id].code;
            }
            token = strtok(NULL, " ");
        }
    } else {
        /* EZTRACE_PAPI_COUNTERS is not defined, use the default values */
        nb_events = 1;
        Events = malloc(sizeof(int) * nb_events);
        Events[0] = PAPI_TOT_INS;
    }
    if (invalid_counters) {
        fprintf(stderr, "Available counters:\n");
        __papi_print_available_counters();
    }
}

static void __papi_init(void) __attribute__ ((constructor));
static void __papi_init(void) {
    int ret;
    pthread_key_create(&papi_thread_info_key, NULL );

    /* initialize PAPI */
    ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret != PAPI_VER_CURRENT && ret > 0) {
        fprintf(stderr, "PAPI library version mismatch!\n");
        exit(1);
    }

    /* There may be several threads, so let's tell this to PAPI */
    if ((ret = PAPI_thread_init(pthread_self)) != PAPI_OK)
        handle_error(ret);

    __papi_init_counter_ids();
    __papi_select_counters();
    papi_init_hw_counter();

    record_hardware_counter = 1;

#ifdef EZTRACE_AUTOSTART
    eztrace_start ();
#endif
    __papi_initialized = 1;

    /* Tell how many counters (and which counters) are monitored */
    EZTRACE_EVENT1(FUT_PAPI_INIT, nb_events);
    int i;
    for (i = 0; i < nb_events; i++)
        EZTRACE_EVENT2(FUT_PAPI_INIT_COUNTERS, i, Events[i]);

}

static void __papi_conclude(void) __attribute__ ((destructor));
static void __papi_conclude(void) {
    __papi_initialized = 0;
    eztrace_stop();
}
