/*	fkt_setmask.c

	fkt_setmask --	program to set current FKT mask

	Copyright (C) 2003, 2004 Samuel Thibault <samuel.thibault@ens-lyon.org>

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
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fkt.h>
#include <string.h>

#define TRYARGV(s,new_todo)	\
	if (!strcmp(argv[0]+strlen(argv[0])-strlen(s),s)) todo=new_todo;

int main(int argc, char **argv) {
	int fd;
	int arg;
	int todo=-1;	/* what to do, depending on how we were called */

	TRYARGV("setmask",FKT_SETMASK);
	TRYARGV("enable",FKT_ENABLE);
	TRYARGV("disable",FKT_DISABLE);
	TRYARGV("probe0",FKT_USER_PROBE0);
	if (todo == -1) {
		fprintf(stderr,"please call me as setmask, enable or disable\n");
		exit(1);
	}
	if (argc<2) {
		fprintf(stderr,"I need the ");
		switch(todo) {
		case FKT_SETMASK:
			printf("mask\n");
			break;
		case FKT_ENABLE:
			printf("bit to enable\n");
			break;
		case FKT_DISABLE:
			printf("bit to disable\n");
			break;
		case FKT_USER_PROBE0:
			printf("code to use\n");
			break;
		}
		exit(2);
	}
	arg=strtol(argv[1],NULL,0);
 	fd = open("/dev/fkt",O_RDONLY);
	if (fd<0) {
		perror("open fkt");
		exit(3);
	}
	if (ioctl(fd,todo,arg)<0) {
		perror("ioctl");
		exit(4);
	}
	return 0;
}
