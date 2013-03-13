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
#include "timer.h"

double pr_times( FILE * fp, clock_t real, struct tms * tmsstart,
											struct tms * tmsend, int verbose )
	{
    static long clktck;
	double	elapsed;

    if (clktck == 0)
        if((clktck = sysconf(_SC_CLK_TCK)) < 0)
            perror("sysconf error");

	elapsed = ((double)real)/((double)clktck);
	if( verbose )
		{
		fprintf(fp, "Elapsed: %7.2f, ", elapsed);
		fprintf(fp, "user: %7.2f, ", (tmsend->tms_utime - tmsstart->tms_utime) /
			   (double) clktck);
		fprintf(fp, "sys:  %7.2f\n", (tmsend->tms_stime - tmsstart->tms_stime) /
			   (double) clktck);
		}
	return elapsed;
	}


void err_dump( char *s, int n )
	{
    fprintf(stderr, "%s: %s\n", s, strerror(n));
    exit(EXIT_FAILURE);
	}


int cnvint( char *text )
	{
	int		n;
	char	*ptr;

	errno = 0;
	n = strtol(text, &ptr, 10);
	if( errno != 0 )
		{/* probably number to convert is outside range */
		perror(text);
		exit(EXIT_FAILURE);
		}
	if( strspn(ptr, WHITE_SPACE) != strlen(ptr) )
		{/* garbage in the text string that couldn't be converted */
		fprintf(stderr, "%s: illegal integer number\n", text);
		exit(EXIT_FAILURE);
		}
	return n;
	}


void fpr_times( FILE * fp, int amount, clock_t real)
	{
    static long clktck;

    if (clktck == 0)
        if((clktck = sysconf(_SC_CLK_TCK)) < 0)
            perror("sysconf error");

    fprintf(fp, "%d\t", amount);
    fprintf(fp, "%7.2f\n", real / (double) clktck);
	}

/* vim: ts=4
 */
