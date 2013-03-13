/*	fkt_print.c

	fkt_print --	program to analyse and print trace files produced by
					fkt_record using fkt (fast kernel tracing) in the kernel.

	Copyright (C) 2000, 2001  Robert D. Russell -- rdr@unh.edu
	2003, 2004, 2012  Samuel Thibault -- samuel.thibault@ens-lyon.org

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


*/

/*	As of 30-Jul-03,
 *
	unshifted codes (for traps/syscalls/irqs) MUST be less than
		FKT_UNSHIFTED_LIMIT_CODE.

	singleton codes (i.e., those that are NOT paired in entry-exit pairs)
		MUST be greater than FKT_UNPAIRED_LIMIT_CODE
		and be less thant GCC_TRACED_FUNCTION_X86_MINI
		FKT_SETUP_CODE and less than FKT_LAST_FKT_CORE.

	function entry and exit codes MUST be greater than
		FKT_UNSHIFTED_LIMIT_CODE.
	function entry codes MUST have the FKT_GENERIC_EXIT_OFFSET bit set.

	gcc traced functions codes are greater than GCC_TRACED_FUNCTION_X86_MINI
		entry codes must have GCC_TRACED_FUNCTION_X86_EXIT bit set
*/

#include <config.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#ifndef __MINGW32__
#include <sys/ioctl.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "fxt.h"
#include "fxt_internal.h"
#include "fxt-tools.h"

#define CALLOC(t,size) \
        do \
        if( (t = calloc(size, sizeof(typeof(*t)))) == NULL ) \
                { \
                fprintf(stderr, "unable to calloc "#t"[%d]\n", size); \
                exit(EXIT_FAILURE); \
                } \
        while (0);

/*	use 64-bit arithmetic for all times */
typedef unsigned long long u_64;

/*
 *
 * Settings variables
 *
 */

/*
 * lib fxt structure
 */
static fxt_t fxt;
static struct fxt_infos *info;

/*
 * 
 * Some limits and their actual use
 *
 */

static size_t		page_size;
int trace_space;

static const char* trace_space_name[] = { 
	[FXT_SPACE_KERNEL]="Kernel traces",
	[FXT_SPACE_USER]="User traces",
};


//static struct pid_cycles_item	*pid_cycles_list;
static int			pid;
static int			kidpid;

/*	basetime is first absolute time ever seen */
/*	lasttime[thiscpu] is last  absolute time ever seen for thiscpu */
/*	lastreltime[thiscpu] is last relative time ever seen for thiscpu */
/*	should have lastreltime = lasttime - basetime */
/*	basehigh is the current 32bit hi part of the 64bit time */
//static u_64		basetime;
static u_64             *lasttime;
static u_64		*basehigh;
static u_64		*lastreltime;

/*	start_time_cpu[thiscpu] is 1st relative time for thiscpu once taking stats*/
/*	end_time_cpu[thiscpu] is 1st relative time for thiscpu after taking stats */
static u_64		*start_time_cpu, *end_time_cpu;

/*	already_saw[thiscpu] is 0 before any slot for thiscpu is seen, else 1 */
/*	fun_time_cpu[thiscpu] is 1 after thiscpu has started taking stats,
							-1 after thiscpu has finished taking stats */
static int			*already_saw, *fun_time_cpu;
static int			*lastpid;
//static clock_t		start_jiffies;
static clock_t                  stop_jiffies;
static int			fd;

/* this gets incremented each time we charge a a function with
 * cycles, to detect incoherencies as soon as possible */
//static u_64			accounted;


/*
bufptr[0] is the first item for each line ie cycles
bufptr[1] is pid in low half, processor number in high half
bufptr[2] is code
bufptr[3] is P1
bufptr[4] is P2
...
bufptr[n] is P(n-2)
*/

/*
 *
 * dump one slot
 *
 */

int trace_format;

#include "fut.h"
struct fxt_code_name fut_code_table [] =
    {
#  if !defined(PREPROC) && !defined(DEPEND)
#    include "fut_print.h"
#  endif
    {0, NULL }
    };
#  define find_name(code, keep, maxlen) ({ \
    const char *_res; \
    if (code >= FUT_I386_FUNCTION_MINI) \
        _res=fxt_lookup_symbol(fxt,code&~FUT_I386_FUNCTION_EXIT); \
    else \
        _res=fxt_find_name(fxt, code, keep, maxlen, fut_code_table); \
    _res; \
    })


#if 0
unsigned long *dumpslot( unsigned long *bufptr )
{

	switch (fxt->events_block.ondisk.type) {
	case FXT_BLOCK_TRACES_USER_RAW_NATIVE:
	{
		int i;
		struct fxt_trace_user_raw32*ev = (typeof(ev)) bufptr;
		int nb_args=((ev->code&0xFF)-sizeof(*ev))/sizeof(long);
		long code=ev->code>>8;
	 	if (nb_args < 0) {
			printf("Argh: %i args\n", nb_args);
			nb_args=0;
		}
	 	if (nb_args > 4) {
			printf("Argh: %i args\n", nb_args);
			nb_args=4;
		}
		printf("%"PRIu64"u\t[0x%16lx]\t%16lx(%i)", ev->tick, ev->tid,
			 ev->code, nb_args);
#  define GCC_INSTRUMENT_ENTRY_CODE FUT_GCC_INSTRUMENT_ENTRY_CODE
#  define GCC_INSTRUMENT_EXIT_CODE FUT_GCC_INSTRUMENT_EXIT_CODE
#  define GCC_TRACED_FUNCTION_X86_EXIT FUT_I386_FUNCTION_EXIT

		if (code == GCC_INSTRUMENT_ENTRY_CODE ||
             		code == GCC_INSTRUMENT_EXIT_CODE) {
			const char *s;
			printf(" %25s", code==GCC_INSTRUMENT_ENTRY_CODE?
                        "gcc-traced function entry":"gcc-traced function exit");
			code=ev->args[0];
			s=fxt_lookup_symbol(fxt,code);
			printf("  %s",s);
			printf("\n");
		} else {
			printf(" %25s", fxt_find_name(fxt, code, 1, 25, fut_code_table));
			for (i=0; i< nb_args; i++) {
				printf("\t%"PRIu64"x", ev->args[i]);
			}
			printf("\n");
		}
		return &ev->args[nb_args];
	}
	default:
		error_format("thiscpu");
	}

	return NULL;
}

#endif

void get_the_time( time_t the_time, char *message )
	{
	struct tm	*breakout;
	char		buffer[128];


	if( (breakout = localtime(&the_time)) == NULL )
		{
		fprintf(stderr, "Unable to break out %s\n", message);
		exit(EXIT_FAILURE);
		}
	if( strftime(buffer, 128, "%d-%b-%Y %H:%M:%S", breakout) == 0 )
		{
		fprintf(stderr, "Unable to convert %s\n", message);
		exit(EXIT_FAILURE);
		}
	printf("%14s = %s\n", message, buffer);
	}

void dump_native(fxt_blockev_t evs, int offset, int delta) {
	struct fxt_ev_native ev;
	int ret;
	static uint64_t time_offset=0;

	while (FXT_EV_OK == (ret=fxt_next_ev(evs, FXT_EV_TYPE_NATIVE, (struct fxt_ev *)&ev))) {
		int i;
		if (offset) {
			if (time_offset==0) {
				time_offset=ev.time;
				ev.time=0;
			} else {
				ev.time -= time_offset;
			}
		} else if (delta) {
			ev.time -= time_offset;
			time_offset += ev.time;
		}
		printf("%20"PRIu64"u\t[%16lx]\t%2u\t%16lx(%i)\t", ev.time, (long)ev.user.tid, ev.cpu,
		       (long)ev.code, ev.nb_params);
		
#  define GCC_INSTRUMENT_ENTRY_CODE FUT_GCC_INSTRUMENT_ENTRY_CODE
#  define GCC_INSTRUMENT_EXIT_CODE FUT_GCC_INSTRUMENT_EXIT_CODE
#  define GCC_TRACED_FUNCTION_X86_EXIT FUT_I386_FUNCTION_EXIT
		if (ev.code == GCC_INSTRUMENT_ENTRY_CODE ||
             		ev.code == GCC_INSTRUMENT_EXIT_CODE) {
			const char *s;
			unsigned long func;
			printf("%25s", ev.code==GCC_INSTRUMENT_ENTRY_CODE?
                        "gcc-traced function entry":"gcc-traced function exit");
			func=ev.param[0];
			s=fxt_lookup_symbol(fxt,func);
			printf("\t%s",s);
			printf("\n");
		} else if (ev.code == FUT_SET_THREAD_NAME_CODE) {
			char *name=(char *)(ev.param+1);
			printf("%25s\t%s\n", fxt_find_name(fxt, ev.code, 1,
							    25,fut_code_table),
			       name);			
		} else {
			printf("%25s", fxt_find_name(fxt, ev.code, 1, 25, fut_code_table));
			for (i=0; i< ev.nb_params && i < FXT_MAX_PARAMS; i++) {
				printf("\t%lx", ev.param[i]);
			}
			printf("\n");
		}

	}
	if (ret != FXT_EV_EOT) {
		printf("Stopping on code %i\n", ret);
	}
}

void dump64(fxt_blockev_t evs, int offset, int delta) {
	struct fxt_ev_64 ev;
	int ret;

	while (FXT_EV_OK == (ret=fxt_next_ev(evs, FXT_EV_TYPE_64, (struct fxt_ev *)&ev))) {
		int i;
		printf("%"PRIu64"u\t[%16lx]\t%2u\t%16lx(%i)", ev.time, (long)ev.user.tid, ev.cpu,
		       (long)ev.code, ev.nb_params);
		
		//printf(" %25s", fxt_find_name(fxt, code, 1, 25, fut_code_table));
		for (i=0; i< ev.nb_params && i < FXT_MAX_PARAMS; i++) {
			printf("\t%"PRIu64"x", ev.param[i]);
		}
		printf("\n");

	}
	if (ret != FXT_EV_EOT) {
		printf("Stopping on code %i\n", ret);
	}
}

#define OPTIONS ":f:dox"
#ifndef WHITE_SPACE
#define WHITE_SPACE " \t\n\f\v\r"
#endif

void usage(char *progname)
{
	printf("Usage: %s [-d] [-o] [-x] [-f record-file]\n\
\n\
	if -f is not given, look for TRACE_FILE environment variable, and\n\
		if that is not given, read from \"" DEFAULT_TRACE_FILE "\".\n\
\n\
        -d : dump mode (no trace interpretation)\n\
        -o : time offset (from begining)\n\
	-x : time delta (between events)\n\
", progname);
}

int main( int argc, char *argv[] )
{
	char			*infile;
	int				c;
	struct stat		st;
	off_t			off=0;
	size_t			size;
	int                     dump=0;
	int                     time_offset=0, time_delta=0;

	kidpid = 0;
	infile = NULL;
	opterr = 0;

	while( (c = getopt(argc, argv, OPTIONS)) != EOF ) {
		switch( c )
		{
		case 'f':
			infile = optarg;
			break;
		case 'd':
			dump=1;
			break;
		case 'o':
			time_offset=1;
			break;
		case 'x':
			time_delta=1;
			break;
		case ':':
			fprintf(stderr, "missing parameter to switch %c\n", optopt);
			usage(argv[0]);
			exit(EXIT_FAILURE);
		case '?':
			fprintf(stderr, "illegal switch %c\n", optopt);
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}	/* switch */
	}
	
	if( optind < argc )
		fprintf(stderr, "extra command line arguments ignored\n");
	
	if( infile == NULL )
	{
		if( (infile = getenv("TRACE_FILE")) == NULL )
			infile = DEFAULT_TRACE_FILE;
	}
	
	while( (fd = open(infile, O_RDONLY)) < 0 )
	{/* could not open the indicated file name for reading */
		perror(infile);
		if( strcmp(infile, DEFAULT_TRACE_FILE) == 0 )
			/* can't open the default file, have to give up */
			exit(EXIT_FAILURE);
		infile = DEFAULT_TRACE_FILE;
		fprintf(stderr, "input defaulting to %s\n", infile);
	}
	
	if (!(fxt = fxt_fdopen(fd)))
		{
		perror("fxt_fdopen");
		exit(EXIT_FAILURE);
		}
	info = fxt_infos(fxt);

	printf("%14s = %i\n", "ncpus", info->ncpus);
	
	if (info->space==FXT_SPACE_KERNEL) {
		CALLOC(basehigh,info->ncpus);
	}
	CALLOC(lasttime,info->ncpus);
	CALLOC(lastreltime,info->ncpus);
	CALLOC(start_time_cpu,info->ncpus);
	CALLOC(end_time_cpu,info->ncpus);
	CALLOC(lastpid,info->ncpus);
	CALLOC(already_saw,info->ncpus);
	CALLOC(fun_time_cpu,info->ncpus);
	
	pid = info->record_pid;
	kidpid = info->traced_pid;
	
	get_the_time(info->start_time, "start time");
	get_the_time(info->stop_time, "stop time");

	printf("%14s = %lu\n", "start jiffies", (unsigned long)info->start_jiffies);
	printf("%14s = %lu\n", "stop jiffies", (unsigned long)info->stop_jiffies);
	printf("%14s = %lu\n", "total jiffies", (unsigned long)info->stop_jiffies -
	       (unsigned long)info->start_jiffies);
	
	page_size = info->page_size;
	printf("%14s = %zu\n", "page size", page_size);
	trace_space = info->space;
	printf("%14s = %u (%s)\n", "trace space", trace_space, trace_space_name[trace_space]);
	printf("%s\n", info->uname);
	
	if (fstat(fd, &st)) {
		perror("fstat");
		exit(EXIT_FAILURE);
	}
	
#ifdef BLKGETSIZE
	if( S_ISBLK(st.st_mode) )
	{
		int sectors;
		if( ioctl(fd,BLKGETSIZE,&sectors) )
		{
			perror("getting device size\n");
			exit(EXIT_FAILURE);
		}
		if( sectors >= 0xffffffffUL >> 9 )
			size=0xffffffffUL;
		else
			size = sectors << 9;
	}
	else
#endif
		size=st.st_size;
	
	printf("%14s = %llu\n", "bytes", (unsigned long long) (size-off));
	
	if( size > off )
	{
		fxt_blockev_t evs;
		evs=fxt_blockev_enter(fxt);
		if (dump) {
			dump64(evs, time_offset, time_delta);
		} else {
			dump_native(evs, time_offset, time_delta);
		}
	}
	
	if (stop_jiffies)
	{/* fkt_record could write the stop time, good */
		printf("\n");
	}
	
	return EXIT_SUCCESS;
}

