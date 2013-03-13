/*
 * Copyright (C) 2009, 2012  Université de Bordeaux 1
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

/* Wrapper to avoid msys' tendency to turn / into \ and : into ;  */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	char *prog, *arch, *def, *name, *lib;
	char s[1024];

	if (argc != 6)
	{
		fprintf(stderr,"bad number of arguments");
		exit(EXIT_FAILURE);
	}

	prog = argv[1];
	arch = argv[2];
	def = argv[3];
	name = argv[4];
	lib = argv[5];

	snprintf(s, sizeof(s), "\"%s\" /machine:%s /def:%s /name:%s /out:%s",
		 prog, arch, def, name, lib);
	if (system(s))
	{
		fprintf(stderr, "%s failed\n", s);
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
