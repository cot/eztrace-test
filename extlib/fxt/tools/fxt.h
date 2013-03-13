/*
 *  Copyright (C) 2004, 2011 Samuel Thibault <samuel.thibault@ens-lyon.org>
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the program ; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __FXT_H
#define __FXT_H
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <stdint.h>

/******************************************************************************
 *
 * fxt_t holds every information found in blocks, as well as pointers on the
 * trace
 */

typedef struct _fxt *fxt_t;

/******************************************************************************
 *
 * opening a trace file
 */

/* open a trace file, given its name */
extern fxt_t fxt_open(const char *path);
/* the same, but for an already-opened trace file (via a pipe etc) */
extern fxt_t fxt_fdopen(int fd);

/* close the trace file */
extern void fxt_close(fxt_t fxt);

/******************************************************************************
 *
 * tools for trace recorders:
 */
#define FXT_TRACE_KERNEL_RAW32 0
#define FXT_TRACE_KERNEL_RAW64 1
#define FXT_TRACE_USER_RAW32 2
#define FXT_TRACE_USER_RAW64 3
typedef struct fxt_evbuf {
	char __fxt_internal[16]; /* must match block header size */
	long data[];
} *fxt_evbuf_t;
extern fxt_t fxt_setinfos(unsigned int space);
extern int fxt_fdwrite(fxt_t fxt, int fd);
extern int fxt_fdwritetime(fxt_t fxt, int fd);
extern int fxt_fdevents_start(fxt_t fxt, int fd, int kind);
extern int fxt_fdevents_stop(fxt_t fxt, int fd);
extern int fxt_setupeventsbuffer(fxt_t fxt, struct fxt_evbuf *buffer, long size, int kind);
extern int fxt_fdwriteevents(fxt_t fxt, int fd, struct fxt_evbuf *buffer);

/******************************************************************************
 *
 * misc informations
 */

struct fxt_infos {
#define FXT_SPACE_KERNEL 0
#define FXT_SPACE_USER   1
	unsigned int space;

/* Little-endian machines */
#define FXT_ARCH_I386 0
#define FXT_ARCH_IA64 1
#define FXT_ARCH_X86_64 2

/* Big-endian machines */
#define FXT_ARCH_PPC 0x10000
#define FXT_ARCH_PPC64 0x20000
	uint32_t arch;     /* so that syscalls/irqs/... are known */

	unsigned int ncpus;
	unsigned int *mhz;	/* ncpus array */

	pid_t record_pid;	/* recording process pid */
	pid_t traced_pid;	/* traced process pid */

	time_t start_time;
	time_t stop_time;

	clock_t start_jiffies;
	clock_t stop_jiffies;

	unsigned long page_size;

	const char *uname;
};

struct fxt_infos *fxt_infos(fxt_t fxt);

/******************************************************************************
 *
 * irq information
 */
extern int fkt_fill_irqs(fxt_t fxt);
extern const char *fkt_find_irq(fxt_t fxt, unsigned long code);

extern int fkt_find_syscall(fxt_t fxt, unsigned long code, char *category, const char **name, int *should_be_hex);

/******************************************************************************
 *
 * symbols information
 */
typedef enum {
	FXT_SYSMAP_FILE,
	FXT_MODLIST,
	FXT_PROC,
	FXT_USER,
} fxt_symbol_source;

struct fxt_code_name {
	unsigned long code;
	const char *name;
};

#define FKT_I386_NTRAPS	21
#define FKT_I386_NSYSIRQS	18
#define FKT_I386_NSYSCALLS	283

extern const char *fkt_i386_traps[FKT_I386_NTRAPS];
extern const char *fkt_i386_sysirqs[FKT_I386_NSYSIRQS];
extern const char *fkt_i386_syscalls[FKT_I386_NSYSCALLS];

extern struct fxt_code_name fkt_code_table[];

extern const char *fxt_find_name(fxt_t fxt, unsigned long code, int keep_entry, int maxlen, struct fxt_code_name *table);

extern void fxt_get_symbols(fxt_t fxt, const char *file, fxt_symbol_source source, unsigned long base);
extern void fut_get_mysymbols(fxt_t fxt);
extern int fxt_load_symbols(fxt_t fxt, FILE *fstream);
extern const char *fxt_lookup_symbol(fxt_t fxt, unsigned long address);

/******************************************************************************
 *
 * Codes space
 */

#define FKT_I386_FUNCTION_MINI 0x80000000ULL
#define FKT_I386_FUNCTION_EXIT 0x40000000ULL

#define FUT_I386_FUNCTION_MINI 0x00400000ULL
#define FUT_I386_FUNCTION_EXIT 0x80000000ULL

#define FKT_GCC_TRACED_FUNCTION_X86_MINI 0x80000000UL
#define FKT_GCC_TRACED_FUNCTION_X86_EXIT 0x40000000UL

#define FUT_GCC_TRACED_FUNCTION_X86_MINI 0x00400000UL
#define FUT_GCC_TRACED_FUNCTION_X86_EXIT 0x80000000UL

/******************************************************************************
 *
 * pids information
 */

#define FXT_MAXCOMMLEN 15
extern int fkt_add_pid(fxt_t fxt, uint64_t pid, const char name[FXT_MAXCOMMLEN+1]);
extern void fkt_remove_pid(fxt_t fxt, uint64_t pid);
extern const char *fxt_lookup_pid(fxt_t fxt, uint64_t pid);

/******************************************************************************
 *
 * Trace events
 */
#include <stdint.h>
#define FXT_MAX_PARAMS 8
#define FXT_MAX_DATA (FXT_MAX_PARAMS*sizeof(uint64_t))

/* TODO: only keep ev_64 */
struct fxt_ev_native {
	uint64_t time;
	union {
		struct {
			unsigned int /*pid_t*/ pid;
		} kernel;
		struct {
			unsigned long tid;
		} user;
	};
	unsigned int cpu;
	unsigned long code;
	unsigned int nb_params;
	unsigned long param[FXT_MAX_PARAMS];
	unsigned char raw[FXT_MAX_DATA];
};

struct fxt_ev_32 {
	uint64_t time;
	union {
		struct {
			uint32_t pid;
		} kernel;
		struct {
			uint32_t tid;
		} user;
	};
	uint32_t cpu;
	uint32_t code;
	unsigned int nb_params;
	uint32_t param[FXT_MAX_PARAMS];
	unsigned char raw[FXT_MAX_DATA];
};

struct fxt_ev_64 {
	uint64_t time;
	union {
		struct {
			uint32_t pid;
		} kernel;
		struct {
			uint64_t tid;
		} user;
	};
	uint32_t cpu;
	uint64_t code;
	unsigned int nb_params;
	uint64_t param[FXT_MAX_PARAMS];
	unsigned char raw[FXT_MAX_DATA];
};

#define FXT_EV_TYPE_NATIVE 0
#define FXT_EV_TYPE_32 1
#define FXT_EV_TYPE_64 2
struct fxt_ev {
	union {
		struct fxt_ev_native native;
		struct fxt_ev_32 ev32;
		struct fxt_ev_64 ev64;
	};
};

/* represents the event stream */
typedef struct fxt_blockev *fxt_blockev_t;
/* enter the event stream of the trace file */
fxt_blockev_t fxt_blockev_enter(fxt_t fxt);
#define FXT_EV_OK 0
#define FXT_EV_ERROR -1
#define FXT_EV_EOT 1 /* End Of Trace */
#define FXT_EV_TYPEERROR 2 /* on essaie de forcer une trace 64 dans un
			    * type 32 */
/* get next event in the event stream */
int fxt_next_ev(fxt_blockev_t evs, int ev_type, struct fxt_ev *ev);
/* rewind within the event stream */
int fxt_rewind(fxt_blockev_t fxt);

// fonctions avancées: savoir le type d'évènement et en extraire des infos:
//
// TIME_OFFSET -> 64 bits (32hi << 32)
//
// SWITCH_TO -> next
// FUNC_ENTER -> identifiant de la fonction puis fonction d'impression
// FUNC_EXIT
//
// ENTER_KERNEL  (irq, syscall) -> id_enter
// EXIT_KERNEL -> ()
//
// NEW_ENTITY -> nouveau pid
// SET_ENTITY_NAME -> nom du processus
// END_ENTITY -> pid tué
// BIND_USER_ENTITY -> utid
//
// OTHER
//
// fonctions.
//
//
// id fn -> nom
// id enter -> nom, type(irq(numéro)/syscall(nom))
//
// trace user / noyau ?

// deux fichiers différents pour infos & trace -> pouvoir ajouter des infos
// ids en début de blocks
// 
// format de trace: usertid enregistré ou pas, 64/32 bits
// little/big endian
//
// numéro du processeur, rajouter dans struct ev à la lecture
//
// premier block: assurément params de la trace
//
// le reste ordre quelconque, id.
//
//
// la trace est dans un bloc (pas terminé)
//

#include <stdint.h>

/* Structures utilisées par le code enregistrant les traces (et ev.c
 * pour lire ces traces) */

struct fxt_trace_kernel_raw32 {
        uint64_t tick;
        uint16_t cpu;
        uint16_t pid;
        uint32_t code;
        uint32_t args[];
};

struct fxt_trace_user_raw32 {
        uint64_t tick;
        uint32_t tid;
        uint32_t code;
        uint32_t args[];
};

struct fxt_trace_user_raw64 {
        uint64_t tick;
        uint64_t tid;
        uint64_t code;
        uint64_t args[];
};

#undef FXT_ARCH_64
#undef FXT_ARCH_32
#ifdef __i386
#  define FXT_RECORD_ARCH FXT_ARCH_I386
#  define FXT_BLOCK_TRACES_KERNEL_RAW FXT_BLOCK_TRACES_KERNEL_RAW32
#  define FXT_BLOCK_TRACES_USER_RAW FXT_BLOCK_TRACES_USER_RAW32
#  define FXT_BLOCK_TRACES_USER_RAW_NATIVE FXT_BLOCK_TRACES_USER_RAW32
#  define FXT_ARCH_32
typedef struct fxt_trace_user_raw32 fxt_trace_user_raw_t;
typedef struct fxt_trace_kernel_raw32 fxt_trace_kernel_raw_t;

#elif defined __ia64
#  define FXT_ARCH_64
#  define FXT_RECORD_ARCH FXT_ARCH_IA64;
#  define FXT_BLOCK_TRACES_KERNEL_RAW FXT_BLOCK_TRACES_KERNEL_RAW64
#  define FXT_BLOCK_TRACES_USER_RAW FXT_BLOCK_TRACES_USER_RAW64
#  define FXT_BLOCK_TRACES_USER_RAW_NATIVE FXT_BLOCK_TRACES_USER_RAW64
typedef struct fxt_trace_user_raw64 fxt_trace_user_raw_t;

#elif defined __x86_64__
#  define FXT_RECORD_ARCH FXT_ARCH_X86_64;
#  define FXT_BLOCK_TRACES_KERNEL_RAW FXT_BLOCK_TRACES_KERNEL_RAW64
#  define FXT_BLOCK_TRACES_USER_RAW FXT_BLOCK_TRACES_USER_RAW64
#  define FXT_BLOCK_TRACES_USER_RAW_NATIVE FXT_BLOCK_TRACES_USER_RAW64
#  define FXT_ARCH_64
typedef struct fxt_trace_user_raw64 fxt_trace_user_raw_t;

#elif defined(__powerpc__) || defined(__ppc__) || defined(_ARCH_PPC)
#  define FXT_RECORD_ARCH FXT_ARCH_PPC;
#  define FXT_BLOCK_TRACES_KERNEL_RAW FXT_BLOCK_TRACES_KERNEL_RAW32
#  define FXT_BLOCK_TRACES_USER_RAW FXT_BLOCK_TRACES_USER_RAW32
#  define FXT_BLOCK_TRACES_USER_RAW_NATIVE FXT_BLOCK_TRACES_USER_RAW32
#  define FXT_ARCH_32
typedef struct fxt_trace_user_raw32 fxt_trace_user_raw_t;

#elif defined(__PPC64__) || defined(__powerpc64__)
#  define FXT_RECORD_ARCH FXT_ARCH_PPC64;
#  define FXT_BLOCK_TRACES_KERNEL_RAW FXT_BLOCK_TRACES_KERNEL_RAW64
#  define FXT_BLOCK_TRACES_USER_RAW FXT_BLOCK_TRACES_USER_RAW64
#  define FXT_BLOCK_TRACES_USER_RAW_NATIVE FXT_BLOCK_TRACES_USER_RAW64
#  define FXT_ARCH_64
typedef struct fxt_trace_user_raw64 fxt_trace_user_raw_t;

#else
#  error Unable to find a supported host architecture.
#endif

#ifdef FXT_ARCH_32
#  define FXT_TRACE_KERNEL_RAW FXT_TRACE_KERNEL_RAW32
#  define FXT_TRACE_USER_RAW FXT_TRACE_USER_RAW32
#else /* FXT_ARCH_64 */
#  define FXT_TRACE_KERNEL_RAW FXT_TRACE_KERNEL_RAW64
#  define FXT_TRACE_USER_RAW FXT_TRACE_USER_RAW64
#endif

#endif /* __FXT_H */
