/*	ev.c

	ev -- events management.

	Copyright (C) 2000, 2001  Robert D. Russell -- rdr@unh.edu

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

/*****	15-Oct-00	rdr		modified for 2.4.0-test9 *****/
/*	also commented out use of FKT_TCP_RECVMSG_SCHEDI_CODE */

#include <config.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include "fxt.h"
#include "fut.h"
#include "linux/fkt.h"
#include "fxt-tools.h"
#include "fxt_internal.h"

struct fxt_blockev {
	/* fichier contenant les traces */
	fxt_t fxt;
	/* types d'événements (FXT_TRACE_...) */
	long type;
	/* block disque contenant les événements */
	struct fxt_block block;

	/* position courante dans le fichier
	 * A étoffer en lisant le fichier par block...
	 */
	off_t cur_pos;
	
	/* Données particulières pour chaque type de trace */
	union {
		struct {
			/* offset de fin des données (après il faut
			 * aller à la page suivante) */
			off_t end_data;
			/* offset du début de la page de données
			   (null à l'initialisation) */
			off_t start_data;
		} raw;
	};
};

static int fxt_fill_blockev(fxt_t fxt, fxt_blockev_t evs)
{
	off_t offset=0;
	FILE *fstream=evs->fxt->fstream;
	fxt_fblock_enter(fxt, fstream, &evs->block);

	switch (GET_BLOCK_TYPE(evs->block)) {
	case FXT_BLOCK_TRACES_KERNEL_RAW32:
		evs->type=FXT_TRACE_KERNEL_RAW32;
		break;
	case FXT_BLOCK_TRACES_KERNEL_RAW64:
		evs->type=FXT_TRACE_KERNEL_RAW64;
		break;
	case FXT_BLOCK_TRACES_USER_RAW32:
		evs->type=FXT_TRACE_USER_RAW32;
		break;
	case FXT_BLOCK_TRACES_USER_RAW64:
		evs->type=FXT_TRACE_USER_RAW64;
		break;
	default:
		fprintf(stderr,"Unknown block trace type %u\n", GET_BLOCK_TYPE(evs->block));
		return -1;
	}

	switch (evs->type) {
	case FXT_TRACE_KERNEL_RAW32:
	case FXT_TRACE_KERNEL_RAW64:
 	        READ(64, offset);
		/* Fall thru */
	case FXT_TRACE_USER_RAW32:
	case FXT_TRACE_USER_RAW64:
		if ((fseek(fstream,offset,SEEK_CUR))<0) {
			perror("seeking to start of events");
			return -1;
		}
		evs->cur_pos=ftell(fstream);
		break;
	}
	switch (evs->type) {
	case FXT_TRACE_KERNEL_RAW32:
	case FXT_TRACE_KERNEL_RAW64:
		evs->raw.end_data  =evs->cur_pos;
		evs->raw.start_data=evs->cur_pos - evs->fxt->infos.page_size;
		break;
	case FXT_TRACE_USER_RAW32:
	case FXT_TRACE_USER_RAW64:
	        evs->raw.end_data  = evs->block.start + evs->block.ondisk.size;
		evs->raw.start_data=evs->cur_pos;
		break;
	}
	return 0;
}

fxt_blockev_t fxt_blockev_enter(fxt_t fxt)
{
	fxt_blockev_t evs;

	if (!(evs = malloc(sizeof(*evs)))<0) goto out;
	evs->fxt=fxt;

	if (fxt_fill_blockev(fxt, evs) < 0) goto outmalloc;

	return evs;
 outmalloc:
	free(evs);
 out: 
	return NULL;
}

#define NOT_MANAGED \
    fprintf(stderr, "Error: trace type %li not yet managed\n", evs->type); \
    abort()

int fxt_next_ev(fxt_blockev_t evs, int ev_type, struct fxt_ev *ev)
{
	fxt_t fxt = evs->fxt;
	FILE *fstream = fxt->fstream;
	int size_ev=0;
	int i;
	int ret=0;
	struct fxt_ev_64 e;

#define READEV_VAR(size, var)			\
	{ READ(size, var);	\
	  size_ev += size/8 ;			\
	}

#define READEV(size, field) \
	READEV_VAR(size, e.field)

	switch (evs->type) {
	case FXT_TRACE_KERNEL_RAW32:
	case FXT_TRACE_KERNEL_RAW64:
	case FXT_TRACE_USER_RAW32:
	case FXT_TRACE_USER_RAW64:
		/* Est-on encore dans la trace ? */
		if (evs->cur_pos >= evs->raw.end_data) {
			/* Est-ce la fin ? */
			if (evs->cur_pos!=0 && (evs->type == FXT_TRACE_USER_RAW32 ||
						evs->type == FXT_TRACE_USER_RAW64)) {
				return FXT_EV_EOT;
			}
						
			/* On repositionne cur_pos */
			evs->cur_pos = evs->raw.start_data+fxt->infos.page_size;
			evs->raw.start_data=evs->cur_pos;
			if (fseek(fstream,evs->cur_pos,SEEK_SET)!=0) {
				fprintf(stderr, "Unable to go to next page at offset %"PRIu64"u\n", evs->cur_pos);
				return 1;
			}
			switch (evs->type) {
			case FXT_TRACE_KERNEL_RAW32:
			{
				uint32_t u32;
				int res;
				res = fread(&u32, sizeof(u32), 1, fxt->fstream);
				SWAP(32,u32);
				if (res < 0)
					return -1;
				if (res < 1 /* FIXME: pas forcément fin de fichier */
					|| u32 == FKT_EOF)
					return FXT_EV_EOT;
				if (!u32) {
					fprintf(stderr,"null size !\n");
					return 1;
				}
				if (u32 == FKT_POISON) {
					fprintf(stderr,"poisoned size !\n");
					return 1;
				}
				if (u32>fxt->infos.page_size) {
					fprintf(stderr,"big page size ! %#010x\n",u32);
					return 1;
				}
				evs->raw.end_data = evs->raw.start_data + u32;
				evs->cur_pos += 32/8;
			}
			}
		}
		break;
	default: NOT_MANAGED;		
	}

	/* L'événement est présent, on peut le lire dans 'e' */
	switch (evs->type) {
	case FXT_TRACE_KERNEL_RAW32:
		READEV(32,time);
		READEV(16, kernel.pid);
		READEV(16, cpu);
		READEV(32, code);
		break;

	case FXT_TRACE_KERNEL_RAW64:
		READEV(32,time);
		READEV(64, code);
		break;
	case FXT_TRACE_USER_RAW32:
		READEV(64,time);
		READEV(32, user.tid);
		READEV(32, code);
		break;
	case FXT_TRACE_USER_RAW64:
		READEV(64,time);
		READEV(64, user.tid);
		READEV(64, code);
		break;
	default:
		NOT_MANAGED;
	}

	/* calcul du nombre de paramètres */
	switch (evs->type) {
	case FXT_TRACE_KERNEL_RAW32:
	case FXT_TRACE_KERNEL_RAW64:
		if( e.code < FKT_UNSHIFTED_LIMIT_CODE ) {
			e.nb_params = 0;
		} else {
			switch (evs->type) {
			case FXT_TRACE_KERNEL_RAW32:
				if( e.code & 3 )
					fprintf(stderr, "code & 0x3 != 0 !!!\n");
				e.nb_params = (e.code & 0xff) / 4;
				break;
			case FXT_TRACE_KERNEL_RAW64:
				if( e.code & 7 )
					fprintf(stderr, "code & 0x3 != 0 !!!\n");
				e.nb_params = (e.code & 0xff) / 8;
				break;
			}
			e.code >>= 8;
		}
		break;
	case FXT_TRACE_USER_RAW32:
	case FXT_TRACE_USER_RAW64:
		e.nb_params=e.code & 0xff;
		if (e.nb_params == 0) {
			fprintf(stderr, "code & 0xff == 0 !!!\n");
		} else {
			e.nb_params--;
		}
		if (e.nb_params > FXT_MAX_PARAMS) {
		        fprintf(stderr, "warning: %u arguments in trace (code %"PRIx64") whereas only room for %i\n",
				e.nb_params, e.code, FXT_MAX_PARAMS);
		}
		e.code >>= 8;
		break;
	default:
		NOT_MANAGED;
	}
	/* lecture des paramètres */
	for (i=0;i<e.nb_params;i++) {
		switch (evs->type) {
		case FXT_TRACE_KERNEL_RAW32:
		case FXT_TRACE_USER_RAW32: {
			uint32_t param;
			READRAW(32,param);
			if ((i+1)*4<=FXT_MAX_DATA)
				memcpy(&e.raw[i*4],&param,4);
			if (i<FXT_MAX_PARAMS)
				e.param[i]=param;
     			size_ev += 4;
			break;
		}
		case FXT_TRACE_KERNEL_RAW64:
		case FXT_TRACE_USER_RAW64: {
			uint64_t param;
			READRAW(64,param);
			if ((i+1)*8<=FXT_MAX_DATA)
				memcpy(&e.raw[i*8],&param,8);
			if (i<FXT_MAX_PARAMS)
				e.param[i]=param;
     			size_ev += 8;
			break;
		}
		default:
			NOT_MANAGED;
		}
	}
	switch (evs->type) {
	case FXT_TRACE_KERNEL_RAW32:
	case FXT_TRACE_USER_RAW32:
		if (i*4 < FXT_MAX_DATA)
			memset(&e.raw[i*4],0,FXT_MAX_DATA-i*4);
		break;
	case FXT_TRACE_KERNEL_RAW64:
	case FXT_TRACE_USER_RAW64:
		if (i*8 < FXT_MAX_DATA)
			memset(&e.raw[i*8],0,FXT_MAX_DATA-i*8);
		break;
	default:
		NOT_MANAGED;
	}

	switch (evs->type) {
	case FXT_TRACE_USER_RAW32:
	case FXT_TRACE_USER_RAW64: {
			int thislwp = 0;
			if (e.code == FUT_NEW_LWP_CODE)
				thislwp = e.param[0];
			if (e.code == FUT_NEW_LWP_CODE || fxt->nlwps == 0) {
				if (thislwp >= fxt->nlwps) {
					fxt->nlwps = thislwp+1;
					fxt->secondlasttid = realloc(fxt->secondlasttid, fxt->nlwps*sizeof(*fxt->secondlasttid));
					fxt->lasttid = realloc(fxt->lasttid, fxt->nlwps*sizeof(*fxt->lasttid));
					fxt->already_saw = realloc(fxt->already_saw, fxt->nlwps*sizeof(*fxt->already_saw));
				}
				fxt->lasttid[thislwp] = e.user.tid;
				fxt->already_saw[thislwp] = 1;
			} else {
#ifdef MA__FUT_RECORD_TID
				for (thislwp=0; thislwp<fxt->nlwps; thislwp++) {
					if (fxt->already_saw[thislwp] && fxt->lasttid[thislwp] == e.user.tid)
						break;
				}
				if (thislwp == fxt->nlwps) {
					for (thislwp=0; thislwp<fxt->nlwps; thislwp++) {
						if (fxt->already_saw[thislwp] && fxt->secondlasttid[thislwp] == e.user.tid)
							break;
					}
					if (thislwp == fxt->nlwps) {
						fprintf(stderr,"warning: couldn't find cpu for tid %"PRIx64"\n",e.user.tid);
						thislwp = -1;
					}
				}
				if (thislwp >=0 && e.code == FUT_SWITCH_TO_CODE) {
					fxt->secondlasttid[thislwp] = 
						fxt->lasttid[thislwp];
					fxt->lasttid[thislwp] = e.param[0];
				}
#else
				thislwp = -1;
#endif
			}
			e.cpu = thislwp;
		}
	}

	switch (ev_type) {
#ifdef FXT_ARCH_32
	case FXT_EV_TYPE_NATIVE:
#endif
	case FXT_EV_TYPE_32:
		switch (evs->type) {
		case FXT_TRACE_KERNEL_RAW32:
		case FXT_TRACE_USER_RAW32:
#	define COPY(field)   ev->ev32.field=e.field
			COPY(time);
			switch (evs->type) {
			case FXT_TRACE_KERNEL_RAW32:
				COPY(kernel.pid);
				break;
			case FXT_TRACE_USER_RAW32:
				COPY(user.tid);
				break;
			}
			COPY(cpu);
			COPY(code);
			COPY(nb_params);
			for (i=0;i<e.nb_params && i<FXT_MAX_PARAMS;i++) {
				COPY(param[i]);
			}
			memcpy(ev->ev32.raw,e.raw,sizeof(e.raw));
#	undef COPY
			break;
		case FXT_TRACE_KERNEL_RAW64:
		case FXT_TRACE_USER_RAW64:
			ret=FXT_EV_TYPEERROR;
			break;
		default:
			NOT_MANAGED;
		}
		break;
#ifdef FXT_ARCH_64
	case FXT_EV_TYPE_NATIVE:
#	define COPY(field)   ev->native.field=e.field
		COPY(time);
		switch (evs->type) {
		case FXT_TRACE_KERNEL_RAW64:
			COPY(kernel.pid);
			break;
		case FXT_TRACE_USER_RAW64:
			COPY(user.tid);
			break;
		}
		COPY(cpu);
		COPY(code);
		COPY(nb_params);
		for (i=0;i<e.nb_params && i<FXT_MAX_PARAMS;i++) {
			COPY(param[i]);
		}
		memcpy(ev->native.raw,e.raw,sizeof(e.raw));
#	undef COPY
		break;
#endif
	case FXT_EV_TYPE_64:
		ev->ev64=e;
		break;
	default:
		fprintf(stderr, "warning: unknown event type %i\n", ev_type);
		abort();
	}
	evs->cur_pos += size_ev;
	return ret;
}

int fxt_rewind(fxt_blockev_t evs) {
	if (fseek(evs->fxt->fstream,evs->block.start,SEEK_SET)!=0) {
		perror("seeking to start of events");
		return -1;
	}
	return fxt_fill_blockev(evs->fxt, evs);
}
