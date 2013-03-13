#include "../config.h"

#ifdef MARCEL
#  define MARCEL_INTERNAL_INCLUDE
#  include "marcel.h"
#else
#  ifdef USE_GETTID
#    include <unistd.h>
#    include <sys/syscall.h>	/* For SYS_xxx definitions */
#  endif
#endif

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef HAVE_CLOCK_GETTIME
#include <time.h>
#else
/* to get an efficient implementation of the TBX_GET_TICK on various archs */
#include "tbx_timing.h"
#endif

#ifdef __MINGW32__
#include <windows.h>
#endif

#define tbx_unlikely(x) __builtin_expect(!!(x), 0)

#include "fxt.h"
#include "fxt_internal.h"
#include "fut.h"

extern int record_tid_activated;

#ifdef PROFILE_NEW_FORMAT

/* pseudo fonction pour trouver où écrire un événement dans les buffers */

/*
 * Structure du buffer de trace
 *
 * ADDR (a<b<c...)
 * ||
 * \/
 * 
 * a /--------------- 1ère zone
 *     struct record
 *        last_event = c
 *        start = a
 *        end = e
 *
 * b   ev1
 *
 *     ev2
 *
 *     ...
 *
 * c   evn
 *
 * d   FREE SPACE
 * 
 * e /--------------- 2ième zone
 *     struct record
 *        last_event = g
 *        start = e
 *        end = i
 *        
 * f   ev1
 *
 *     ...
 *
 * g   evn
 *
 * h   FREE SPACE
 *
 * i /--------------- 3ième zone
 *   ...
 * 
 * */


// Variables globales :
void *start_buffer;
void *pos_buffer;
void *end_buffer;

FXT_PERTHREAD_DEFINE(struct fxt_perthread, fxt_perthread_data)

#define SIZE_BUFF 200 /* au hazard */

int __attribute__((no_instrument_function)) 
fut_header (unsigned long code, ... ) 
{
	long size=code && 0xFF;
  
	th_buff old_th, new_th;
  
  restart:
 	/* lecture atomique des 2 valeurs (8 octets) */
 	read8(th_buff, old_th);

	new_th = old_th;
	new_th.pos += size;

	if (new_th.pos > new_th.infos->end) {
     		/* les infos ne tiennent pas dans la place du buffer 
		 * On cherche un nouveau buffer/record */
		void* old_pos = pos_buffer;
		
		void* new_pos = old_pos + SIZE_BUFF;

		if (new_pos > end_buffer) {
			/* On stoppe tout comme actuellement */
			...

			return -E_NOSPACELEFT;
		}
		if (!lock_cmpxchg(pos_buffer, old_pos, new_pos)) {
			/* Des choses ont changés, peut-être nous qui avons été
			 * interrompu et avons alors déjà réexécuté ce code
			 * (donc donné une nouvelle zone à ce thread */
			goto restart;
		}
		/* Ok, on est propriétaire de la nouvelle région qui va de
		 * old_pos à new_pos */
		
		/* On prévoit de la place pour :
		 * 1) les données de controle (struct record)
		 * 2) notre événement
		 * */
		new_th.pos=old_pos+sizeof(record_t)+size;
		
		*((struct record*)old_pos) = {
			.start=old_pos;
			.end=new_pos;
			.last_event=old_pos;
		};
		
		/* Sur ia32, on a cmpxchg8,
		 * mais sur ia64, on a seulement cmp4xchg8 (en fait cmp8xchg16
		 * vu qu'on est en 64 bits)
		 * J'écris donc l'algo avec le plus contraignant
		 *
		 * cmp4xchg8 : compare 4 octets, mais en cas d'égalité on en écrit 8
		 * */
		if (!cmp4xchg8(th_buff.pos, old_th.pos, new_th)) {
			/* Arghhh, on a été interrompu par une autre mesure qui
			 * a mis en place un nouveau buffer
			 * */
			if (lock_cmpxchg(pos_buffer, new_pos, old_pos)) {
				/* On redonne la zone qu'on vient d'allouer si
				 * c'est encore possible */
				goto restart;
			}
			/* Bon, ben on a une zone avec une mesure et pas grand
			 * chose d'autre à mettre dedans...
			 * Tant pis
			 *
			 * cas (*) (voir TODO)
			 * */
		}
	}
	/* Écriture des données à l'adresse :
	 * new_th.pos - size
	 * */
	...
  restart_update:
	/* Et mise à jour du pointeur sur la dernière écriture de la zone */
	int old_last_ev=new_th.infos->last_event;
	if (new_th.pos > old_last_ev) {
		if (!cmpxchg(new_th.infos->last_event, old_last_ev, new_th.pos)) {
			goto restart_update;
		}
	}
	return 0;
}




#else /* PROFILE_NEW_FORMAT */

#ifndef MARCEL
#ifdef __MINGW32__
#ifdef __i386__
  #define ma_cmpxchg(ptr,o,n) \
	InterlockedCompareExchange((ptr),(n),(o))
#elif defined (__x86_64__)
  #define ma_cmpxchg(ptr,o,n) \
	InterlockedCompareExchange64((ptr),(n),(o))
#else
#error TODO test the long size
#endif
#else
/* poor man's cmpxchg, à supprimer lorsque tbx implémentera cmpxchg lui-même */
#define ma_cmpxchg(ptr,o,n) \
	__sync_val_compare_and_swap((ptr), (o), (n))
#endif
#endif

static unsigned long trash_buffer[8];
extern int allow_fut_flush;
extern pthread_mutex_t fut_flush_lock;

#ifdef __GNUC__
__attribute__((weak))
#endif
uint64_t fut_getstamp(void)
{
	unsigned long long tick;
#ifdef HAVE_CLOCK_GETTIME
	struct timespec tp;
#  ifdef CLOCK_MONOTONIC_RAW
	/* avoid NTP mess */
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
#  else
	clock_gettime(CLOCK_MONOTONIC, &tp);
#  endif
	tick = 1000000000ULL*tp.tv_sec+ tp.tv_nsec;
#else
	TBX_GET_TICK(tick);
#endif
	return tick;
}

unsigned long* __fut_record_event(fxt_trace_user_raw_t * rec,
					 uint64_t stamp,
					 unsigned long code)
{
	rec->tick = stamp;
#ifdef MA__FUT_RECORD_TID
	rec->tid=(unsigned long)MARCEL_SELF;
#else
	if (record_tid_activated) {
#  ifdef USE_GETTID
		rec->tid = syscall(SYS_gettid);
#  else
#ifdef __MINGW32__
		rec->tid = GetCurrentThreadId();
#else
		rec->tid = pthread_self();
#endif
#  endif
	} else {
		rec->tid = 0;
	}
#endif
	rec->code = code;
	return (unsigned long*)(rec->args);
}

unsigned long* fut_getstampedbuffer(unsigned long code, 
				    int size)
{
	unsigned long *prev_slot, *next_slot;
	uint64_t stamp;
 retry:
#if 1
	/* compare and swap method */
	do {
		prev_slot=(unsigned long *)fut_next_slot;
		next_slot=(unsigned long*)((unsigned long)prev_slot+size);
	} while (prev_slot != ma_cmpxchg(&fut_next_slot, 
					 prev_slot, next_slot));
	stamp = fut_getstamp();
#else
	/* spin lock method: guarantees coherency between stamp and trace
	 * order, but is much less scalable. */
	pthread_spin_lock(&fut_slot_lock);
	stamp = fut_getstamp();
	prev_slot = fut_next_slot;
	fut_next_slot = next_slot = prev_slot + size;
	pthread_spin_unlock(&fut_slot_lock);
#endif

	if (tbx_unlikely(next_slot > fut_last_slot)) {
	  if(allow_fut_flush) {
	    /* flush the buffer to disk */

	    if (prev_slot > fut_last_slot)
	      /* another thread is going to flush the buffer */
	      goto retry;

	    pthread_mutex_lock(&fut_flush_lock);

	    if ((prev_slot + size > fut_last_slot)
		&& (prev_slot <= fut_last_slot)) {

	      fut_flush(NULL, prev_slot, 1);
	      __fut_reset_pointers();
	    }

	    pthread_mutex_unlock(&fut_flush_lock);

	    goto retry;
	  } else {
	    /* stop recording events */
	    fut_active=0;
	    /* Pourquoi on restaure ici ? Pas de race condition possible ? */
	    fut_next_slot = prev_slot;
	    return trash_buffer;
	  }
	}

	fxt_trace_user_raw_t *rec=(fxt_trace_user_raw_t *)prev_slot;

	return __fut_record_event(rec, stamp, code);
}

#endif /* PROFILE_NEW_FORMAT */
