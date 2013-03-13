/*	fut_setup.c */
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


/*	fut = Fast User Tracing */

#define CONFIG_FUT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#ifndef __MINGW32__
#include <sys/times.h>
#endif
#include <pthread.h>
#include "fxt.h"
#include "fut.h"
#include "fxt-tools.h"
#include "fxt_internal.h"
//#include "pm2_fxt-tools.h"

#ifdef MARCEL
#  include "sys/marcel_flags.h"
#endif

#define MAXCPUS 16

#define UNAMESTRLEN 256


/*	active masks should never become negative numbers in order to prevent
	problems returning them as function results (they look like error codes!) */
#define FULL_ACTIVE_MASK	0x7fffffff

/*	3 external ints */

/*	set to non-zero when probing is active */
volatile unsigned int fut_active = 0;

/*	points to next unused byte in buffer (multiple of 4) */
unsigned long * volatile fut_next_slot = NULL;

/*	points to byte beyond end of buffer (multiple of 4) */
unsigned long * volatile fut_last_slot = NULL;

/*	Protects the fut_*_slot pointers */
pthread_spinlock_t fut_slot_lock;

fxt_t fut;
static struct fxt_infos *fut_infos;

/*	points to region of allocated buffer space */
static char	*bufptr = NULL;

/*	number of contiguous allocated bytes pointed to by bufptr */
static unsigned int	nallocated = 0;

/* filename of the output trace */
static char *fut_filename = NULL;
/* file descriptor of the output trace */
static int fut_fd = 0;

/* if !=0, activate the recording of thread id */
int record_tid_activated = 0;

/* if !=0, activate automatic buffer dumping: when the buffer is full, it is dumped to disk.
 * This permits to record trace larger than the buffer, but this may kill the performance
 */
int allow_fut_flush = 0;

pthread_mutex_t fut_flush_lock;

static int already_flushed = 0;

void fut_set_filename(char* filename)
{
  if(fut_filename) {
    if(already_flushed)
      fprintf(stderr, "Warning: change output file name to %s, but some events have been saved in file %s\n",
	      filename, fut_filename);
    free(fut_filename);
  }
  fut_filename = strdup(filename);
}

void __fut_reset_pointers()
{
  fut_next_slot = (unsigned long *)bufptr;
}

void dumptime( time_t *the_time, clock_t *the_jiffies)
{
#ifndef __MINGW32__
  struct tms cur_time;
#endif

  if( (*the_time = time(NULL)) == -1 )
    perror("time");

#ifdef __MINGW32__
  *the_jiffies = 0;
#else
  if( (*the_jiffies = times(&cur_time)) == (clock_t) -1 )
    perror("times");
#endif
}


/* activate the recording of thread id */
void fut_enable_tid_logging()
{
  record_tid_activated = 1;
}

/* disactivate the recording of thread id */
void fut_disable_tid_logging()
{
  record_tid_activated = 0;
}

/* activate automatic buffer dumping: when the buffer is full, it is dumped to disk.
 * This permits to record trace larger than the buffer, but this may kill the performance
 */
void enable_fut_flush()
{
  allow_fut_flush = 1;
}

/* disactivate automatic buffer dumping
 */
void disable_fut_flush()
{
  allow_fut_flush = 0;
}

/*	called once to set up tracing.
	includes mallocing the buffer to hold the trace.
	returns number of bytes allocated if all ok, else a negative error code.
*/
int fut_setup( unsigned int nlongs, unsigned int keymask, unsigned int threadid )
	{
	unsigned int	nbytes;
	unsigned long	*iptr;


	/* paranoia, so nobody waits for us while we are in here */
	fut_active = 0;

	fut = fxt_setinfos(FXT_SPACE_USER);
	fut_infos = fxt_infos(fut);

	/*	remember pid of process that called setup */
	fut_infos->record_pid = getpid();

	if( bufptr != NULL )
		{/* previous allocation region was not released, do it now */
		free(bufptr);
		/* nothing allocated now */
		bufptr = NULL;
		nallocated = 0;
		}

	/*	allocate buffer */
	nbytes = nlongs * sizeof(long);		/* force multiple of 4/8 bytes */
	if( (bufptr = (char *)malloc(nbytes)) == NULL )
		return -ENOMEM;
	nallocated = nbytes;
	nlongs = nbytes / sizeof(long);

	/* We really touch memory to avoid valgrind's warning afterwards in
	 * case the room actually used to store an event is not a multiple of
	 * size(long), so that there could be some memory that is never
	 * actually initialized. This also avoids page faults while tracing. */
	memset(bufptr, 0, nbytes);

	fut_last_slot = (unsigned long *)(bufptr + nbytes);

	/*	touch all pages so they get loaded */
	for( iptr = (unsigned long *)bufptr;  iptr < fut_last_slot;  )
		{
		*iptr = 0;
		iptr += 0x100;
		}

	dumptime(&fut_infos->start_time, &fut_infos->start_jiffies);

	fut_next_slot = (unsigned long *)bufptr;
	pthread_spin_init(&fut_slot_lock, 0);
	fut_active = keymask & FULL_ACTIVE_MASK;

	FUT_PROBE1(FUT_GCC_INSTRUMENT_KEYMASK,
		FUT_GCC_INSTRUMENT_ENTRY_CODE,
		fut_setup);
	FUT_PROBE3(-1, FUT_SETUP_CODE, fut_active, threadid, nlongs);
	FUT_PROBE0(-1, FUT_CALIBRATE0_CODE);
	FUT_PROBE0(-1, FUT_CALIBRATE0_CODE);
	FUT_PROBE0(-1, FUT_CALIBRATE0_CODE);
	FUT_PROBE1(-1, FUT_CALIBRATE1_CODE, 1);
	FUT_PROBE1(-1, FUT_CALIBRATE1_CODE, 1);
	FUT_PROBE1(-1, FUT_CALIBRATE1_CODE, 1);
	FUT_PROBE2(-1, FUT_CALIBRATE2_CODE, 1, 2);
	FUT_PROBE2(-1, FUT_CALIBRATE2_CODE, 1, 2);
	FUT_PROBE2(-1, FUT_CALIBRATE2_CODE, 1, 2);

	if(allow_fut_flush) {
	  pthread_mutex_init(&fut_flush_lock, NULL);
	}

	return nlongs;
	}


/*	called repeatedly to restart tracing.
	returns previous value of fut_active if all ok, else a negative error code.
*/
int fut_keychange( int how, unsigned int keymask, unsigned int threadid )
	{
	unsigned int	old_active = fut_active;
#if 0
	FUT_PROBE1(FUT_GCC_INSTRUMENT_KEYMASK,
		FUT_GCC_INSTRUMENT_ENTRY_CODE,
		&fut_keychange);
#endif

	if( fut_next_slot >= fut_last_slot  ||  bufptr == NULL )
		return -EPERM;

	switch( how )
		{
	case FUT_ENABLE:
		fut_active |= keymask & FULL_ACTIVE_MASK;
		break;
	case FUT_DISABLE:
		fut_active &= (~keymask) & FULL_ACTIVE_MASK;
		break;
	case FUT_SETMASK:
		fut_active = keymask & FULL_ACTIVE_MASK;
		break;
	default:
		return -EINVAL;
		}

	FUT_PROBE2(-1, FUT_KEYCHANGE_CODE, fut_active, threadid);
#if 0
	printf("Recording keychange %p\n", &fut_keychange);
	FUT_PROBE1(FUT_GCC_INSTRUMENT_KEYMASK,
                   FUT_GCC_INSTRUMENT_EXIT_CODE,
                   &fut_keychange);
#endif
	return old_active;
	}

/*	called once when completely done with buffer.
	returns previous value of active if all ok, else a negative error code.
*/
int fut_done( void )
	{
	unsigned int	old_active = fut_active;

	fut_active = 0;
	if( bufptr != NULL )
		{/* previous allocation region was not released, do it now */
		free(bufptr);
		bufptr = NULL;			/* nothing allocated now */
		nallocated = 0;
		}

	return old_active;
	}


/*	called to reset tracing to refill the entire buffer again.
	returns number of bytes in buffer if all ok, else a negative error code.
*/
int fut_reset( unsigned int keymask, unsigned int threadid )
	{
	unsigned int	nlongs;


	/* paranoia, so nobody waits for us while we are in here */
	fut_active = 0;

	if( bufptr == NULL  ||  nallocated == 0 )
		{/* buffer was never allocated, return error */
		return -ENOMEM;
		}

	/* reset the buffer to completely empty */
	fut_last_slot = (unsigned long *)(bufptr + nallocated);
	fut_next_slot = (unsigned long *)bufptr;
	fut_active = keymask & FULL_ACTIVE_MASK;
	nlongs = nallocated / sizeof(long);

	FUT_PROBE3(-1, FUT_RESET_CODE, fut_active, threadid, nlongs);
	FUT_PROBE0(-1, FUT_CALIBRATE0_CODE);
	FUT_PROBE0(-1, FUT_CALIBRATE0_CODE);
	FUT_PROBE0(-1, FUT_CALIBRATE0_CODE);
	FUT_PROBE1(-1, FUT_CALIBRATE1_CODE, 1);
	FUT_PROBE1(-1, FUT_CALIBRATE1_CODE, 1);
	FUT_PROBE1(-1, FUT_CALIBRATE1_CODE, 1);
	FUT_PROBE2(-1, FUT_CALIBRATE2_CODE, 1, 2);
	FUT_PROBE2(-1, FUT_CALIBRATE2_CODE, 1, 2);
	FUT_PROBE2(-1, FUT_CALIBRATE2_CODE, 1, 2);

	return nlongs;
	}


/*	called repeatedly to copy current buffer into user space.
	returns nlongs >= 0 if all ok, else a negative error code.
*/
int fut_getbuffer( int *nlongs, unsigned long **buffer, unsigned long *next_slot )
	{
	int				local_nlongs = 0;


	if( bufptr == NULL )
		{
		local_nlongs = -EPERM;
		goto out;
		}

	/*	get number of ints worth of trace data currently in the buffer */
	local_nlongs = ((char *)next_slot - bufptr) / sizeof(long);

	if( nlongs != NULL )
		*nlongs = local_nlongs;

	if( buffer != NULL )
		*buffer = (unsigned long *)bufptr;

out:
	return local_nlongs;
	}

int fut_flush( char* filename, void* next_slot, int record_flush_events )
{

  int n, nlongs=0;
  long size;
  unsigned long *copy=NULL;
  static off_t begin = 0;

  dumptime(&fut_infos->stop_time,&fut_infos->stop_jiffies);

  if( (n = fut_getbuffer(&nlongs, &copy, next_slot)) < 0 )
    return n;

  size = nlongs * sizeof(long);

  if(!fut_fd) {

    /* First time fut_flush is called. Open the output file and write headers */
    if( fut_filename == NULL )
      fut_filename = filename ;

    if( fut_filename == NULL )
      fut_filename = DEFAULT_TRACE_FILE;

    /* we can't open the file using the O_NONBLOCK option since after a call to write
     * we are going to modify the buffer. If O_NONBLOCK is set, modifying the buffer while
     * the OS is writting it may corrupt the output trace.
     */

    /* todo: implement a pipeline
     * see Alexandre DENIS. "A High Performance Superpipeline Protocol for InfiniBand" in
     * Proceedings of the 17th International Euro-Par Conference for instance
     */
    if( (fut_fd = open(fut_filename, O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0 )
      return fut_fd;

    already_flushed = 1;
    /* write header */
    fxt_fdwrite(fut, fut_fd);

    /* begin a block of events */
    fxt_fdevents_start(fut, fut_fd, FXT_TRACE_USER_RAW);

    if( (begin=lseek(fut_fd,0,SEEK_CUR)) < 0 ) {
      perror("getting block end seek");
      exit(EXIT_FAILURE);
    }

  }

  fxt_trace_user_raw_t start_flush_event;
  if(record_flush_events)
    __fut_record_event(&start_flush_event, fut_getstamp(), ((FUT_START_FLUSH_CODE)<<8) | 0x01);

  /* dump the buffer to disk */
  if( write(fut_fd, (void *)copy, size) < 0 ) {
    perror("write buffer");
  }

  fxt_trace_user_raw_t stop_flush_event;
  if(record_flush_events) {
    __fut_record_event(&stop_flush_event, fut_getstamp(), ((FUT_STOP_FLUSH_CODE)<<8) | 0x01);

    if( write(fut_fd, (void *)&start_flush_event, FUT_SIZE(0)) < 0 ) {
	perror("write buffer");
    }
    if( write(fut_fd, (void *)&stop_flush_event, FUT_SIZE(0)) < 0 ) {
	perror("write buffer");
    }
  }

  fut_infos->page_size = size;

  off_t end;
  if( (end=lseek(fut_fd,0,SEEK_CUR)) < 0 ) {
    perror("getting block end seek");
    exit(EXIT_FAILURE);
  }

  /* Let's call fxt_fdevents_stop so that the data on disk is already correct.
   * if the program crashes and can't manage to call fut_endup, the trace
   * is still readable.
   */
  fxt_fdevents_stop(fut, fut_fd);

  if( (lseek(fut_fd, end,SEEK_SET)) < 0 ) {
    perror("getting block end seek");
    exit(EXIT_FAILURE);
  }

  return nlongs;
}

int fut_endup( char *filename )
	{

	  /* stop all futher tracing */
	  fut_active = 0;

	  /* dump the buffer to disk */
	  int nlongs = fut_flush(filename, fut_next_slot, 0);
	  if(nlongs < 0)
	    return nlongs;

	  /* end a block of events */
	  fxt_fdevents_stop(fut, fut_fd);

	  if( close(fut_fd) < 0 )
	    perror(fut_filename);

	  __fut_reset_pointers();
	  //fut_done(); // Removed by Raymond
	  fut_active = 1;

	  return nlongs;
	}
