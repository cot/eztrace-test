/*
 *  Copyright (C) 2004, 2011-2012  Samuel Thibault <samuel.thibault@ens-lyon.org>
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#ifndef __MINGW32__
#include <sys/utsname.h>
#include <arpa/inet.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include "fxt.h"
#include "fxt-tools.h"
#include "fxt_internal.h"

struct fxt_block *__fxt_block_current=NULL;

fxt_t fxt_setinfos(unsigned int space) {
	fxt_t fxt;
#ifndef __MINGW32__
	struct utsname unameinfo;
#endif
	char *unamestr;
	int n;

	if (!(fxt = calloc(1,sizeof(*fxt))))
		goto out;

	fxt->infos.space=space;

	fxt->infos.arch = FXT_RECORD_ARCH;

	if (fxt_get_cpu_info(fxt)<0)
		goto outfxt;
	fxt->infos.record_pid = getpid();
	fxt->infos.page_size = getpagesize();

#ifdef __MINGW32__
	unamestr=malloc(7+1);
	snprintf(unamestr,7+1,"mingw32");
#else
	if (uname(&unameinfo))
		goto outfxt;
	n = snprintf(NULL,0,"%s %s %s %s %s",
				unameinfo.sysname,
				unameinfo.nodename,
				unameinfo.release,
				unameinfo.version,
				unameinfo.machine);

	if (!(unamestr=malloc(n+1)))
		goto outfxt;
	snprintf(unamestr,n+1,"%s %s %s %s %s",
				unameinfo.sysname,
				unameinfo.nodename,
				unameinfo.release,
				unameinfo.version,
				unameinfo.machine);
#endif

	fxt->infos.uname=unamestr;

	if (space == FXT_SPACE_KERNEL)
		fkt_fill_irqs(fxt);
	return fxt;

	free(unamestr);
outfxt:
	free(fxt);
out:
	return NULL;
}

int fxt_record_time(fxt_t fxt, int fd) {
	int		i;

	WRITE(32,fxt->infos.space);
	WRITE(32,fxt->infos.ncpus);
	for( i = 0;  i < fxt->infos.ncpus;  i++ )
		WRITE(32,fxt->infos.mhz[i]);
	WRITE(32,fxt->infos.record_pid);
	WRITE(32,fxt->infos.traced_pid);
	WRITE(64,fxt->infos.start_time);
	WRITE(64,fxt->infos.stop_time);
	WRITE(64,fxt->infos.start_jiffies);
	WRITE(64,fxt->infos.stop_jiffies);
	WRITE(32,fxt->infos.page_size);
	return 0;
}

int fxt_load_time(fxt_t fxt, FILE *fstream) {
	int		i;

	READ(32,fxt->infos.space);
	READ(32,fxt->infos.ncpus);
	if (!(fxt->infos.mhz = calloc(fxt->infos.ncpus,sizeof(*fxt->infos.mhz))))
		return -1;
	for( i = 0;  i < fxt->infos.ncpus;  i++ )
		READ(32,fxt->infos.mhz[i]);
	READ(32,fxt->infos.record_pid);
	READ(32,fxt->infos.traced_pid);
	READ(64,fxt->infos.start_time);
	READ(64,fxt->infos.stop_time);
	READ(64,fxt->infos.start_jiffies);
	READ(64,fxt->infos.stop_jiffies);
	READ(32,fxt->infos.page_size);
	return 0;
}

int fxt_fdwrite(fxt_t fxt, int fd) {
	int len;
	int ret;

	/* main block */
	WRITE(32,myhtonl(fxt->infos.arch));
	BEGIN_BLOCK(fd, FXT_BLOCK_INFOS);

	BEGIN_BLOCK(fd, FXT_BLOCK_TIME);
	if ((ret = fxt_record_time(fxt,fd))<0) return ret;
	END_BLOCK(fd);
	BEGIN_BLOCK(fd, FXT_BLOCK_IRQS);
	if ((ret = fkt_record_irqs(fxt,fd))<0) return ret;
	END_BLOCK(fd);

	BEGIN_BLOCK(fd, FXT_BLOCK_UNAME);
	len = strlen(fxt->infos.uname);
	WRITE(32,len);
	if ((ret = write(fd,fxt->infos.uname,len))<0) return ret;
	END_BLOCK(fd);

	BEGIN_BLOCK(fd, FXT_BLOCK_ADDRS);
	if ((ret = fxt_record_symbols(fxt,fd))<0) return ret;
	END_BLOCK(fd);

	BEGIN_BLOCK(fd, FXT_BLOCK_PIDS);
	if ((ret = fkt_record_pids(fxt,fd))<0) return ret;
	END_BLOCK(fd);

	/* and close the main block */
	END_BLOCK(fd);

	return 0;
}

int fxt_fdwritetime(fxt_t fxt, int fd) {
	int ret;
	/* rewrite time. beware that record_time's record size shouldn't change in
	 * the meanwhile ! */
	ENTER_BLOCK_TYPE(fd, FXT_BLOCK_INFOS);
	ENTER_BLOCK_TYPE(fd, FXT_BLOCK_TIME);
	if ((ret = fxt_record_time(fxt, fd))<0) return ret;
	LEAVE_BLOCK(fd);
	LEAVE_BLOCK(fd);
	return 0;
}

int fxt_fdevents_start(fxt_t fxt, int fd, int kind)
{
	off_t	 off, dummyoff=0, size;

	/* start block of traces. If not closed, the block is assumed
	 * to end at the end of the file */

	switch (kind) {
	case FXT_TRACE_KERNEL_RAW:
		/* Noyau, ancien format : on crée un block et on se
		 * positionne en se plaçant sur une frontière de page
		 */

		fxt_block_begin(fd, FXT_BLOCK_TRACES_KERNEL_RAW, 0, 0, &fxt->events_block);

		/* seek to a page boundary */
		if( (off = lseek(fd, 0, SEEK_CUR)) < 0 )
		{
			perror("getting seek");
			return -1;
		}
		off += sizeof(off_t); /* offset we will write */ 
		size = ((off+fxt->infos.page_size-1)&(~(fxt->infos.page_size-1)))
			- off;
		WRITE(64,size);
	
		if( (dummyoff = lseek(fd, size, SEEK_CUR)) != (off+size) ) {
			perror("seeking to a page boundary");
			return -1;
		}
		break;
	case FXT_TRACE_USER_RAW:
		/* User, ancien format : on crée un block et on
		 * placera les données dedans
		 */
		fxt_block_begin(fd, FXT_BLOCK_TRACES_USER_RAW, 0, 0, &fxt->events_block);
		break;
	default:
		abort();
	}

	return 0;
}
int fxt_fdevents_stop(fxt_t fxt, int fd)
{
	fxt_block_end(fd, &fxt->events_block);
	return 0;
}

int fxt_setupeventsbuffer(fxt_t fxt, struct fxt_evbuf *buffer, long size, int kind)
{
	struct fxt_block_ondisk* buf=(struct fxt_block_ondisk*)buffer;
	if (sizeof(struct fxt_evbuf) != sizeof(struct fxt_block_ondisk)) {
		abort();
	}
	buf->size=size;
	switch (kind) {
	case FXT_TRACE_USER_RAW:
		buf->type=FXT_BLOCK_TRACES_USER_RAW;
		break;
	default:
		abort();
	}
	buf->subtype=0;
	return 0;
}

#define SYSCALL(op) \
  while ((op) == -1) \
  { \
    if (errno != EINTR) \
      { \
        perror(#op); \
        exit(EXIT_FAILURE); \
      } \
  }
int fxt_fdwriteevents(fxt_t fxt, int fd, struct fxt_evbuf *buffer)
{
	int ret;
	struct fxt_block_ondisk* buf=(struct fxt_block_ondisk*)buffer;
	
	if (buf->size < sizeof(struct fxt_block_ondisk)) {
		abort();
	}
	SYSCALL(ret=write(fd, buf, buf->size));

	if (ret==-1)
		return -1;
	return 0;
}

fxt_t fxt_open(const char *path) {
	int fd;
	if ((fd=open(path,O_RDONLY))<0)
		return NULL;
	return fxt_fdopen(fd);
}

fxt_t fxt_fdopen(int fd) {
	int ret;
	uint32_t len;
	fxt_t fxt;
	FILE *fstream;

	if (!(fxt = calloc(1,sizeof(*fxt)))<0) goto out;
	if (!(fstream = fdopen(fd, "r"))) {
		perror("fdopening trace");
		goto outmalloc;
	}
	if( fread(&fxt->infos.arch,sizeof(fxt->infos.arch),1,fstream) < 1 )
	{
		perror("read arch");
		goto outf;
	}
	fxt->infos.arch = myntohl(fxt->infos.arch);
	ENTER_FBLOCK_TYPE(fstream, FXT_BLOCK_INFOS); /* main block */
	ENTER_FBLOCK_TYPE(fstream, FXT_BLOCK_TIME);
	if ((ret = fxt_load_time(fxt, fstream))<0) goto outmalloc;
	LEAVE_FBLOCK(fstream);
	ENTER_FBLOCK_TYPE(fstream, FXT_BLOCK_IRQS);
	if ((ret = fkt_load_irqs(fxt, fstream))<0) goto outmalloc2;
	LEAVE_FBLOCK(fstream);
	ENTER_FBLOCK_TYPE(fstream, FXT_BLOCK_UNAME);
	if( fread(&len,sizeof(len), 1, fstream) < 1 )
	{
		perror("read size of uname");
		goto outmalloc2;
	}
	SWAP(32,len);
	{
		char uname[len+1];
		
		if( fread(uname,1,len,fstream) < len )
		{
			perror("dumping uname");
			goto outmalloc2;
		}
		uname[len]='\0';
		fxt->infos.uname = strdup(uname);
	}
	LEAVE_FBLOCK(fstream);
	
	ENTER_FBLOCK_TYPE(fstream, FXT_BLOCK_ADDRS);
	if ((ret = fxt_load_symbols(fxt, fstream))<0) goto outmalloc2;
	LEAVE_FBLOCK(fstream);
	ENTER_FBLOCK_TYPE(fstream, FXT_BLOCK_PIDS);
	if ((ret = fkt_load_pids(fxt, fstream))<0) goto outmalloc2;
	LEAVE_FBLOCK(fstream);
	LEAVE_FBLOCK(fstream); /* main block */
	fxt->fd = fd;
	fxt->fstream = fstream;
	return fxt;
outmalloc2:
	free(fxt->infos.mhz);
outf:
	fclose(fstream);
outmalloc:
	free(fxt);
out:
	return NULL;
}

struct fxt_infos *fxt_infos(fxt_t fxt) {
	return &fxt->infos;
}
