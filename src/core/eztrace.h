/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#ifndef __EZTRACE_H__
#define __EZTRACE_H__

/* don't forget this line, otherwise, FUT_DO_PROBE0 is defined to (void) 0 */
#define CONFIG_FUT

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

#include <stddef.h>
#include <stdint.h>
#include <dlfcn.h>
#ifdef USE_GETTID
#include <unistd.h>
#include <sys/syscall.h>	/* For SYS_xxx definitions */
#else
#include <pthread.h>
#endif	/* USE_GETTID */

#ifdef EXTERNAL_FXT
#include "fxt/fxt.h"
#include "fxt/fut.h"
#else
#include "fxt.h"
#include "fut.h"
#endif

#define EZTRACE_API_VERSION 0x00000700

/* start the recording of events */
void eztrace_start();
/* stop recording events and write the trace to the disk */
void eztrace_stop();
/* change the trace filename */
void eztrace_set_filename(char* name);

/* function  used by launcher */
void set_launcher_env();
void unset_launcher_env();
int is_in_launcher();

void eztrace_code0(uint32_t code);
void eztrace_code1(uint32_t code, uint64_t arg1);
void eztrace_code2(uint32_t code, uint64_t arg1, uint64_t arg2);
void eztrace_code3(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3);
void eztrace_code4(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);
void eztrace_code5(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

typedef void (*eztrace_function_t)(void);

/* register a callback to be called when the application calls
 * eztrace_start().
 * This function is only usefull when AUTOSTART is disabled
 */
void eztrace_register_init_routine(eztrace_function_t init_func);

#ifdef USE_PAPI
extern int record_hardware_counter;
extern void papi_record() __attribute__((weak));
#define RECORD_HW_COUNTERS()			\
  {						\
    if(record_hardware_counter && papi_record)	\
      papi_record();				\
  }

#else
#define RECORD_HW_COUNTERS() (void)0
#endif

extern int eztrace_debug_mode;

#define EZT_PRINTF(debug_level, args...) {	\
    if(eztrace_debug_mode >= debug_level)	\
      fprintf(stderr, ##args);			\
  }

#define FUNCTION_ENTRY					\
  {							\
    EZT_PRINTF(1, "Calling [%s]\n", __FUNCTION__);	\
    RECORD_HW_COUNTERS();				\
  }

/* current thread id */
#ifdef USE_GETTID
#define CUR_TID syscall(SYS_gettid)
#else
#define CUR_TID pthread_self()
#endif

#ifdef TID_RECORDING_ENABLED
#define EZTRACE_EVENT0_UNPROTECTED(code) FUT_DO_PROBE0(code);
#define EZTRACE_EVENT1_UNPROTECTED(code, arg1) FUT_DO_PROBE1(code, arg1);
#define EZTRACE_EVENT2_UNPROTECTED(code, arg1, arg2) FUT_DO_PROBE2(code, arg1, arg2);
#define EZTRACE_EVENT3_UNPROTECTED(code, arg1, arg2, arg3) FUT_DO_PROBE3(code, arg1, arg2, arg3);
#define EZTRACE_EVENT4_UNPROTECTED(code, arg1, arg2, arg3, arg4) FUT_DO_PROBE4(code, arg1, arg2, arg3, arg4);
#define EZTRACE_EVENT5_UNPROTECTED(code, arg1, arg2, arg3, arg4, arg5) FUT_DO_PROBE5(code, arg1, arg2, arg3, arg4, arg5);
#else
#define EZTRACE_EVENT0_UNPROTECTED(code) FUT_DO_PROBE1(code, CUR_TID);
#define EZTRACE_EVENT1_UNPROTECTED(code, arg1) FUT_DO_PROBE2(code, CUR_TID, arg1);
#define EZTRACE_EVENT2_UNPROTECTED(code, arg1, arg2) FUT_DO_PROBE3(code, CUR_TID, arg1, arg2);
#define EZTRACE_EVENT3_UNPROTECTED(code, arg1, arg2, arg3) FUT_DO_PROBE4(code, CUR_TID, arg1, arg2, arg3);
#define EZTRACE_EVENT4_UNPROTECTED(code, arg1, arg2, arg3, arg4) FUT_DO_PROBE5(code, CUR_TID, arg1, arg2, arg3, arg4);
#define EZTRACE_EVENT5_UNPROTECTED(code, arg1, arg2, arg3, arg4, arg5) FUT_DO_PROBE6(code, CUR_TID, arg1, arg2, arg3, arg4, arg5);
#endif

/* create an event without any data */
#define EZTRACE_EVENT0(code)			\
  {						\
    EZTRACE_PROTECT {				\
      EZTRACE_PROTECT_ON();			\
      EZTRACE_EVENT0_UNPROTECTED(code);			\
      EZTRACE_PROTECT_OFF();			\
    }						\
 }
/* create an event with one value */
#define EZTRACE_EVENT1(code, arg1)		\
  {						\
    EZTRACE_PROTECT {				\
      EZTRACE_PROTECT_ON();			\
      EZTRACE_EVENT1_UNPROTECTED(code, arg1);		\
      EZTRACE_PROTECT_OFF();			\
    }						\
 }
/* create an event with two value */
#define EZTRACE_EVENT2(code, arg1, arg2)	\
  {						\
    EZTRACE_PROTECT {				\
      EZTRACE_PROTECT_ON();			\
      EZTRACE_EVENT2_UNPROTECTED(code, arg1, arg2);		\
      EZTRACE_PROTECT_OFF();			\
    }						\
 }
/* etc. */
#define EZTRACE_EVENT3(code, arg1, arg2, arg3)	\
  {						\
    EZTRACE_PROTECT {				\
      EZTRACE_PROTECT_ON();			\
      EZTRACE_EVENT3_UNPROTECTED(code, arg1, arg2, arg3);	\
      EZTRACE_PROTECT_OFF();			\
    }						\
  }

#define EZTRACE_EVENT4(code, arg1, arg2, arg3, arg4)	\
  {							\
    EZTRACE_PROTECT {					\
      EZTRACE_PROTECT_ON();				\
      EZTRACE_EVENT4_UNPROTECTED(code, arg1, arg2, arg3, arg4);	\
      EZTRACE_PROTECT_OFF();				\
    }							\
  }

#define EZTRACE_EVENT5(code, arg1, arg2, arg3, arg4, arg5)	\
  {								\
    EZTRACE_PROTECT {						\
      EZTRACE_PROTECT_ON();					\
      EZTRACE_EVENT5_UNPROTECTED(code, arg1, arg2, arg3, arg4, arg5);	\
      EZTRACE_PROTECT_OFF();					\
    }								\
  }

/* check wether dlsym returned successfully */
#define  TREAT_ERROR()				\
  do {						\
    char * error;				\
    if ((error = dlerror()) != NULL)  {		\
      fputs(error, stderr);			\
      abort();					\
    }						\
  }while(0)

/* interception function func and store its previous value into var */
#define INTERCEPT(func, var)					\
  do {								\
    if(var) break;						\
    void *__handle = RTLD_NEXT;					\
    var = (typeof(var)) (uintptr_t) dlsym(__handle, func);	\
    TREAT_ERROR();						\
  } while(0)

/* return the offset of the field MEMBER in a structure TYPE */
#define ezt_offset_of(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/* Find the global structure's address
 * It needs :
 * - ptr: address of intern field
 * - type: type of the global structure
 * - member: name of the intern field of the global structure
 */
#define ezt_container_of(ptr, type, member)			\
  ((type *)((char *)(__typeof__ (&((type *)0)->member))(ptr)-	\
	    ezt_offset_of(type,member)))

/* record an event (code=FUT_CALLING_FUNCTION) with the calling function name */
void record_backtrace();

/* return 1 if an event is being recorded. */
int recursion_shield_on();

void set_recursion_shield_on();
void set_recursion_shield_off();

/* avoid infinite recursion */
#define EZTRACE_PROTECT if(! recursion_shield_on())

/* Set the recursion shield */
#define EZTRACE_PROTECT_ON() set_recursion_shield_on()
#define EZTRACE_PROTECT_OFF() set_recursion_shield_off()

/* pptrace stuff */
#define PPTRACE_START_INTERCEPT char* PPTRACE_HIJACK_FUNCTION[] = {
#define PPTRACE_INTERCEPT_FULL(function, repl, orig) #function " " #orig " " #repl,
#define PPTRACE_INTERCEPT(function) #function " orig_" #function " " #function,
#define PPTRACE_END_INTERCEPT  NULL };

/* check wether dlsym returned successfully */
#define DYNAMIC_INTERCEPTION(func, varName, var) \
  do {								\
	var = (void**) dlsym(NULL, varName); \
	if(NULL == var) { \
		TREAT_ERROR(); \
	} \
    if(NULL != *var) break;						\
    *var = (void*) dlsym((void*)-1l, func);	\
    TREAT_ERROR();						\
  } while(0)

#define DYNAMIC_INTERCEPT_ALL() \
	do { \
	   if (is_in_launcher()) \
	      return; \
		char __buff[1024]; \
		int __i; char *__j; char *__k; void **__sym; \
		for(__i = 0; PPTRACE_HIJACK_FUNCTION[__i] != NULL; __i++) { \
			strncpy(__buff, PPTRACE_HIJACK_FUNCTION[__i], 1024); \
			__buff[1023] = 0; \
			__j = strchr(__buff, ' '); \
			__k = strchr(__j+1, ' '); \
			*__k = 0; *__j = 0; \
			DYNAMIC_INTERCEPTION(__buff, (__j+1), __sym); \
			*__k = *__j = ' '; \
		} \
	} while(0)

#define START_INTERCEPT PPTRACE_START_INTERCEPT
#define END_INTERCEPT PPTRACE_END_INTERCEPT
#define INTERCEPT2(func, var) func " " #var " " func,

#endif	/* __EZTRACE_H__ */
