/*	fut.h */
/*
 * PM2: Parallel Multithreaded Machine
 * Copyright (C) 2001 "the PM2 team" (see AUTHORS file)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */


#ifndef __FUT_H__
#define __FUT_H__

/*	fut = Fast User Tracing */

#ifdef MARCEL
#  include "sys/marcel_flags.h"
#endif
#include <stdarg.h>
#include <stdint.h>

#include "fxt.h"

/*	Macros for use from within the kernel */
#ifndef __ASSEMBLY__
extern volatile unsigned int fut_active;
#endif

#if defined(CONFIG_FUT)

/***********************************************/
/* Les macros 
 *
 * FUT_CODE(code, nb_param) : calcul du code avec d�calage
 *   n�cessaire pour :
 *     FUT_RAW_PROBEx(...)
 *     FUT_*_PROBE(...)    [avec * dans 'RAW' 'DO' 'FULL' et ''] 
 *   mais pas pour :
 *     FUT_*_PROBEx(...)   [avec * dans 'DO' 'FULL' et ''] 
 *
 * FUT_SIZE(nb_param) : calcul de la taille de la structure
 *
 * FUT_RAW_PROBEx(fut_code, ...) : fut_code est d�j� d�cal�
 * FUT_DO_PROBEx(code, ...) : code n'est pas encode d�cal�
 * FUT_FULL_PROBEx(keymask, code, ...) : enregistre si le keymask est actif
 * FUT_PROBEx(keymask, code, ...) : idem, sauf avec CONFIG_FUT_TIME_ONLY 
 *                              (dans ce cas, on enregistre aucun param�tre)
 *
 * FUT_*_PROBE([keymask,] fut_code, ...) : nombre variable d'arguments
 *                                         fut_code est d�j� d�cal�
 */


__BEGIN_TEMPLATE_ARGS_NAME__
NUMBER
DECLARE_ARGS
CALL_ARGS
ARGS_REGISTER
__END_TEMPLATE_ARGS_NAME__

__BEGIN_TEMPLATE_ARGS__
0



__END_TEMPLATE_ARGS__
__BEGIN_TEMPLATE_ARGS__
1
,P1
,P1
*(args++)=(unsigned long)(P1);
__END_TEMPLATE_ARGS__
__BEGIN_TEMPLATE_ARGS__
2
,P1,P2
,P1,P2
*(args++)=(unsigned long)(P1);*(args++)=(unsigned long)(P2);
__END_TEMPLATE_ARGS__
__BEGIN_TEMPLATE_ARGS__
3
,P1,P2,P3
,P1,P2,P3
*(args++)=(unsigned long)(P1);*(args++)=(unsigned long)(P2);*(args++)=(unsigned long)(P3);
__END_TEMPLATE_ARGS__
__BEGIN_TEMPLATE_ARGS__
4
,P1,P2,P3,P4
,P1,P2,P3,P4
*(args++)=(unsigned long)(P1);*(args++)=(unsigned long)(P2);*(args++)=(unsigned long)(P3);*(args++)=(unsigned long)(P4);
__END_TEMPLATE_ARGS__
__BEGIN_TEMPLATE_ARGS__
5
,P1,P2,P3,P4,P5
,P1,P2,P3,P4,P5
*(args++)=(unsigned long)(P1);*(args++)=(unsigned long)(P2);*(args++)=(unsigned long)(P3);*(args++)=(unsigned long)(P4);*(args++)=(unsigned long)(P5);
__END_TEMPLATE_ARGS__
__BEGIN_TEMPLATE_ARGS__
6
,P1,P2,P3,P4,P5,P6
,P1,P2,P3,P4,P5,P6
*(args++)=(unsigned long)(P1);*(args++)=(unsigned long)(P2);*(args++)=(unsigned long)(P3);*(args++)=(unsigned long)(P4);*(args++)=(unsigned long)(P5);*(args++)=(unsigned long)(P6);
__END_TEMPLATE_ARGS__

#define FUT_CODE(CODE, n) ((((unsigned long)(CODE))<<8) | ((n) + 1))
#define FUT_SIZE(n) (sizeof(fxt_trace_user_raw_t)+(n)*sizeof(long))
#define FUT_STRLEN(s) ((strlen(s)+sizeof(long)-1)/sizeof(long))

__BEGIN_TEMPLATE__
#define FUT_RAW_PROBE__NUMBER__(CODE__DECLARE_ARGS__) do {		\
		if(fut_active) {					\
			unsigned long *args __attribute__((unused))=	\
				fut_getstampedbuffer(CODE,		\
						     FUT_SIZE(__NUMBER__)); \
			__ARGS_REGISTER__				\
				}					\
	} while (0)

__END_TEMPLATE__
#define FUT_RAW_PROBESTR(CODE,S) do {					\
		if(fut_active) {					\
			const char *s = (S);				\
			size_t len = strlen(s);				\
			size_t n = FUT_STRLEN(s);			\
			void *args __attribute__((unused)) =		\
				fut_getstampedbuffer(CODE, FUT_SIZE(n)); \
			memcpy(args, s, len);				\
			memset(args+len, 0, n*sizeof(long)-len);	\
		}							\
	} while (0)

__BEGIN_TEMPLATE__
#define FUT_DO_PROBE__NUMBER__(CODE__DECLARE_ARGS__) do { \
        FUT_RAW_PROBE__NUMBER__(FUT_CODE(CODE, __NUMBER__)__CALL_ARGS__); \
} while (0)

__END_TEMPLATE__
#define FUT_DO_PROBESTR(CODE,S) do { \
	FUT_RAW_PROBESTR(FUT_CODE(CODE, FUT_STRLEN(s)), S); \
} while (0)

/******************************************************/
/* NEVER call fut_getstampedbuffer directly           */
/* Interface can change without forewarning           */
/******************************************************/
extern unsigned long* fut_getstampedbuffer(unsigned long code, int size);

/* This can be overriden by the user */
extern uint64_t fut_getstamp(void);

inline static void
#ifndef __cplusplus
__attribute__((no_instrument_function))
#endif
FUT_RAW_PROBE(unsigned long code, ...) {
	if(fut_active) {		
	int nb=(code&0xff)-1;
        unsigned long *args=fut_getstampedbuffer(code, FUT_SIZE(nb));
	va_list list;

	va_start(list, code);
	while (nb--) {
		*(args++)=va_arg(list, unsigned long);
	}
	va_end(list);
	} 
}
#define FUT_DO_PROBE(CODE, ...) do { \
        FUT_RAW_PROBE(CODE , ##__VA_ARGS__); \
} while (0)


__BEGIN_TEMPLATE_ARGS__

,...
 , ##__VA_ARGS__

__END_TEMPLATE_ARGS__

__BEGIN_TEMPLATE__
#define FUT_FULL_PROBE__NUMBER__(KEYMASK,CODE__DECLARE_ARGS__) do { \
	if( KEYMASK & fut_active ) { \
                FUT_DO_PROBE__NUMBER__(CODE__CALL_ARGS__); \
        } \
} while(0)

__END_TEMPLATE__
#define FUT_FULL_PROBESTR(KEYMASK,CODE,S) do { \
	if (KEYMASK & fut_active) { \
		FUT_DO_PROBESTR(CODE,S); \
	} \
} while(0)

#else /* CONFIG_FUT */

__BEGIN_TEMPLATE__
#define FUT_DO_PROBE__NUMBER__(CODE__DECLARE_ARGS__)   do {} while (0)
#define FUT_FULL_PROBE__NUMBER__(MASK,CODE__DECLARE_ARGS__) do {} while (0)
__END_TEMPLATE__
#define FUT_DO_PROBESTR(CODE,S) do {} while (0)
#define FUT_FULL_PROBESTR(MASK,CODE,S) do {} while (0)

#endif

#if defined(CONFIG_FUT_TIME_ONLY)

__BEGIN_TEMPLATE__
#define FUT_PROBE__NUMBER__(KEYMASK,CODE__DECLARE_ARGS__) FUT_FULL_PROBE0(KEYMASK,CODE)
__END_TEMPLATE__
#define FUT_PROBESTR(KEYMASK,CODE,S) FUt_FULL_PROBESTR(KEYMASK,CODE)

#else	/* CONFIG_FUT_TIME_ONLY */

__BEGIN_TEMPLATE__
#define FUT_PROBE__NUMBER__(KEYMASK,CODE__DECLARE_ARGS__) FUT_FULL_PROBE__NUMBER__(KEYMASK,CODE__CALL_ARGS__)
__END_TEMPLATE__
#define FUT_PROBESTR(KEYMASK,CODE,S) FUT_FULL_PROBESTR(KEYMASK,CODE,S)

#endif	/* CONFIG_FUT_TIME_ONLY */


/*	"how" parameter values, analagous to "how" parameters to FKT */
#define FUT_ENABLE		0xCE03		/* for enabling probes with 1's in keymask */
#define FUT_DISABLE		0xCE04		/* for disabling probes with 1's in keymask */
#define FUT_SETMASK		0xCE05		/* for enabling 1's, disabling 0's in keymask */

/*	Simple keymasks */
#define FUT_KEYMASK0				0x00000001
#define FUT_KEYMASK1				0x00000002
#define FUT_KEYMASK2				0x00000004
#define FUT_KEYMASK3				0x00000008
#define FUT_KEYMASK4				0x00000010
#define FUT_KEYMASK5				0x00000020
#define FUT_KEYMASK6				0x00000040
#define FUT_KEYMASK7				0x00000080
#define FUT_KEYMASK8				0x00000100
#define FUT_KEYMASK9				0x00000200
#define FUT_KEYMASK10				0x00000400
#define FUT_KEYMASK11				0x00000800
#define FUT_KEYMASK12				0x00001000
#define FUT_KEYMASK13				0x00002000
#define FUT_KEYMASK14				0x00004000
#define FUT_KEYMASK15				0x00008000
#define FUT_KEYMASK16				0x00010000
#define FUT_KEYMASK17				0x00020000
#define FUT_KEYMASK18				0x00040000
#define FUT_KEYMASK19				0x00080000
#define FUT_KEYMASK20				0x00100000
#define FUT_KEYMASK21				0x00200000
#define FUT_KEYMASK22				0x00400000
#define FUT_KEYMASK23				0x00800000
#define FUT_KEYMASK24				0x01000000
#define FUT_KEYMASK25				0x02000000
#define FUT_KEYMASK26				0x04000000
#define FUT_KEYMASK27				0x08000000
#define FUT_KEYMASK28				0x10000000
#define FUT_KEYMASK29				0x20000000
#define FUT_KEYMASK30				0x40000000
#define FUT_KEYMASK31				0x80000000
#define FUT_KEYMASKALL				0xffffffff

#define FUT_GCC_INSTRUMENT_KEYMASK	FUT_KEYMASK29


/*	Fixed parameters of the fut coding scheme */
#define FUT_GENERIC_EXIT_OFFSET		0x100	/* exit this much above entry */

#define FUT_UNPAIRED_LIMIT_CODE		0xf000	/* all unpaired codes above this limit */

/*	Codes for fut use */
#define FUT_SETUP_CODE				0xffff
#define FUT_KEYCHANGE_CODE			0xfffe
#define FUT_RESET_CODE				0xfffd
#define FUT_CALIBRATE0_CODE			0xfffc
#define FUT_CALIBRATE1_CODE			0xfffb
#define FUT_CALIBRATE2_CODE			0xfffa

#define FUT_THREAD_BIRTH_CODE                   0xfff9
#define FUT_THREAD_DEATH_CODE                   0xfff8
#define FUT_SET_THREAD_NAME_CODE                0xfff7

#define FUT_NEW_LWP_CODE                        0xfff6

#define FUT_START_FLUSH_CODE                    0xfff5
#define FUT_STOP_FLUSH_CODE                     0xfff4

#define FUT_RQS_NEWLEVEL			0xffef
#define FUT_RQS_NEWLWPRQ			0xffee
#define FUT_RQS_NEWRQ				0xffed

#define FUT_SWITCH_TO_CODE			0x31a

#define FUT_MAIN_ENTRY_CODE			0x301
#define FUT_MAIN_EXIT_CODE			0x401

/* -finstrument-functions code */
#define FUT_GCC_INSTRUMENT_ENTRY_CODE	0x320
#define FUT_GCC_INSTRUMENT_EXIT_CODE	0x420

#ifndef __ASSEMBLY__
extern unsigned long * volatile fut_next_slot;
extern unsigned long * volatile fut_last_slot;
extern pthread_spinlock_t fut_slot_lock;

extern int fut_setup( unsigned int nints, unsigned int keymask,
			unsigned int threadid );
extern int fut_endup( char *filename );
extern int fut_done(void );
extern int fut_keychange( int how, unsigned int keymask,
			unsigned int threadid );
extern int fut_reset( unsigned int keymask, unsigned int threadid );
extern int fut_getbuffer( int *nints, unsigned long **buffer, unsigned long *next_slot);

extern int fut_header( unsigned long head, ... );

extern void enable_fut_flush( void );
extern void disable_fut_flush( void );

extern void fut_set_filename( char *filename );

/* activate the recording of thread id */
extern void fut_enable_tid_logging();

/* disactivate the recording of thread id */
extern void fut_disable_tid_logging();
#endif /* __ASSEMBLY__ */

#endif /* __FUT_H__ */
