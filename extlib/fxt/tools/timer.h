/*
 *  Copyright (C) 2000, 2001 Robert Russell <rdr@cs.unh.edu>
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>

#ifndef WHITE_SPACE
#define WHITE_SPACE	" \f\n\r\t\v"
#endif

#define MEGA		(1000000.0)

extern double pr_times( FILE * fp, clock_t real, struct tms * tmsstart,
											struct tms * tmsend, int verbose );

extern void fpr_times(FILE * fp, int amount, clock_t real);

extern void err_dump( char *s, int n );

extern int cnvint( char *text );
