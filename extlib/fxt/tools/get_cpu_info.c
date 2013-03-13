/*
 *  Copyright (C) 2000, 2001  Robert Russell <rdr@cs.unh.edu>
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
#include <string.h>
#include "fxt.h"
#include "fxt_internal.h"

#define BUF_SIZE	4096
#define TAGMHZ1		"cpu MHz"
#define TAGMHZ2		"Clocking"
#define TAGMHZ3		"cpu clock"
#define TAGMHZ4		"clock"
#define TAGMHZ5		"Clock"
#define TAGMHZ6		"CPU clock"
#define TAGHZ1		"ClkTck"
#define TAGHZ2		"cycle frequency [Hz]"
#define TAGHZ3		"CPU-Clock"
#define PROC_FILE	"/proc/cpuinfo"

int fxt_get_cpu_info( fxt_t fxt )
	{
	FILE	*fptr;
	char	*ptr, buffer[BUF_SIZE];
	double	cpu_mhz;
	unsigned long long cpu_hz;

	if (fxt->infos.mhz)
		free(fxt->infos.mhz);
	fxt->infos.ncpus = 0;

	if( (fptr=fopen(PROC_FILE, "r")) == NULL )
		{
		perror("fopen");
		fprintf(stderr, "unable to open %s\n", PROC_FILE);
		fprintf(stderr, "Assuming 1 1MHz proc\n");
		fxt->infos.mhz = malloc(sizeof(*fxt->infos.mhz));
		fxt->infos.mhz[0] = 1;
		return fxt->infos.ncpus = 1;
		}

	for( ; ; )
		{/* once around loop for each line in the file */
		if( fgets(buffer, BUF_SIZE, fptr) == NULL )
			{
			if( feof(fptr) )
				{/* end of the file, did we find any cpus? */
				fclose(fptr);
				if( fxt->infos.ncpus <= 0 )
					/* no, end of file was premature */
					fprintf(stderr, "end of file on %s\n", PROC_FILE);
				else
					/* yes, normal return with that number */
					return fxt->infos.ncpus;
				}
			else
				fprintf(stderr, "unable to read %s\n", PROC_FILE);
			return -1;
			}
		/***** fputs(buffer, stdout); *****/
		if( strncmp(buffer, TAGMHZ1, strlen(TAGMHZ1)) == 0
		  ||strncmp(buffer, TAGMHZ2, strlen(TAGMHZ2)) == 0
		  ||strncmp(buffer, TAGMHZ3, strlen(TAGMHZ3)) == 0
		  ||strncmp(buffer, TAGMHZ4, strlen(TAGMHZ4)) == 0
		  ||strncmp(buffer, TAGMHZ5, strlen(TAGMHZ5)) == 0
		  ||strncmp(buffer, TAGMHZ6, strlen(TAGMHZ6)) == 0 )
			{/* found the line starting with the tag */
			/***** fprintf(stderr, "found line starting with %s\n", TAG); *****/
			if( (ptr = strchr(buffer, ':')) == NULL )
				{
				fprintf(stderr, "cannot find ':' in %s\n", buffer);
				return -1;
				}
			if( sscanf(ptr+1, "%lf", &cpu_mhz) != 1 )
				{
				fprintf(stderr, "cannot find number after ':' in %s\n", buffer);
				return -1;
				}
			/***** fprintf(stderr, "found cpu mhz %.2f\n", cpu_mhz); *****/
			}
		else if( strstr(buffer, TAGHZ1)
		       ||strncmp(buffer, TAGHZ2, strlen(TAGHZ2) == 0)
		       ||strncmp(buffer, TAGHZ3, strlen(TAGHZ3) == 0) )
			{
			if( (ptr = strchr(buffer, ':')) == NULL )
				{
				fprintf(stderr, "cannot find ':' in %s\n", buffer);
				return -1;
				}
			if( sscanf(ptr+1, "%llx", &cpu_hz) != 1 )
				{
				fprintf(stderr, "cannot find number after ':' in %s\n", buffer);
				return -1;
				}
			cpu_mhz = cpu_hz / 1000000;
			}
		else continue;
		if( !(fxt->infos.mhz = realloc(fxt->infos.mhz,(fxt->infos.ncpus+1)*sizeof(*fxt->infos.mhz))) )
			{
			fprintf(stderr, "cannot allocate memory for cpu %u\n",fxt->infos.ncpus);
			return -1;
			}
		fxt->infos.mhz[fxt->infos.ncpus++] = cpu_mhz;
		}
	}

/* vim: ts=4
 */
