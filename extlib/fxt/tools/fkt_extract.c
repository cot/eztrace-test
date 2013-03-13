/*	fkt_extract.c

	fkt_extract --	program to extract a portion of a trace, given by its page
					numbers.

	Copyright (C) 2003 Samuel Thibault <samuel.thibault@ens-lyon.org>

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

#include <config.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/mman.h>

ssize_t slow_copyfile(int out_fd, int in_fd, off_t *offset, size_t count)
	{
	void *buffer;
	size_t done,donebuffer;
	ssize_t res;
	int page_size = getpagesize();
	if( count%page_size )
		{
		fprintf(stderr,"size is not page-aligned\n");
		exit(EXIT_FAILURE);
		}
	if( (buffer = mmap(NULL, page_size, PROT_READ|PROT_WRITE,
								MAP_PRIVATE|MAP_ANONYMOUS,0,0)) == MAP_FAILED
		&& !(buffer = malloc(page_size)))
		{
		perror("allocating page buffer\n");
		exit(EXIT_FAILURE);
		}
	done=0;
	while( done<count )
		{
		donebuffer=0;
		while( donebuffer<page_size )
			{
			if( (res=read(in_fd, buffer+donebuffer, page_size-donebuffer) < 0) )
				{
				if( res != EINTR )
					{
					perror("read");
					exit(EXIT_FAILURE);
					}
				}
			else if( res==0 )
				{
				fprintf(stderr,"unexpected end of file\n");
				exit(EXIT_FAILURE);
				}
			else donebuffer+=res;
			}
		donebuffer=0;
		while( donebuffer<page_size )
			{
			if( (res=write(in_fd, buffer+donebuffer, page_size-donebuffer) < 0) )
				{
				if( res != EINTR )
					{
					perror("write");
					exit(EXIT_FAILURE);
					}
				}
			else donebuffer+=res;
			}
		done+=page_size;
		}
	return done;
	}

ssize_t copyfile(int out_fd, int in_fd, off_t *offset, size_t count)
	{
	size_t done=0;
	ssize_t ret;
	while( done<count )
		{
		if( (ret=sendfile(out_fd, in_fd, offset, count-done)) < 0 )
			{
			if( errno == EINVAL && done==0 )
				return slow_copyfile(out_fd, in_fd, offset, count);
			perror("sendfile");
			exit(EXIT_FAILURE);
			}
		else if( ret == 0 )
			{
			fprintf(stderr,"unexpected end of file\n");
			exit(EXIT_FAILURE);
			}
		else done+=ret;
		}
	return done;
	}

void usage( char *argv0 )
	{
	fprintf(stderr,"\
usage: %s in-file out-file begin end\n\
	where begin and end are page numbers, inclusive\n\
",argv0);
	exit(EXIT_FAILURE);
	}

int main( int argc, char *argv[] )
	{
	char 			*c;
	int				fdin,fdout;
	size_t			begin, end;
	unsigned int	offset;
	off_t			copyoff;
	int				res;
	int				page_size = getpagesize();

	if( argc < 5 )
		usage(argv[0]);

	if( (fdin=open(argv[1],O_RDONLY)) < 0 )
		{
		fprintf(stderr,"couldn't open %s\n",argv[1]);
		perror("open");
		exit(EXIT_FAILURE);
		}

	if( (fdout=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0664)) < 0 )
		{
		fprintf(stderr,"couldn't open %s\n",argv[2]);
		perror("open");
		exit(EXIT_FAILURE);
		}

	begin=strtoll(argv[3],&c,0)*page_size;
	if( c == argv[3] )
		usage(argv[0]);

	end=(strtoll(argv[4],&c,0)+1)*page_size;
	if( c == argv[4] )
		usage(argv[0]);

	if( (res=read(fdin, &offset, sizeof(offset))) < sizeof(offset) )
		{
		if( res<0 )
			perror("read offset");
		else
			fprintf(stderr,"Unexpected end of file\n");
		exit(EXIT_FAILURE);
		}
	if( offset <= 32 || offset%page_size )
		{
		fprintf(stderr,"incorrect offset\n");
		exit(EXIT_FAILURE);
		}


	lseek(fdin,0,SEEK_SET);

	/* copy header */
	copyoff=0;
	copyfile(fdout,fdin,&copyoff,offset);
	/* copy data */
	copyoff=offset+begin;
	copyfile(fdout,fdin,&copyoff,end-begin);
	/* and that's it :) */
	return EXIT_SUCCESS;
	}

/* vim: ts=4
 */
