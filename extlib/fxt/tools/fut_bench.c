/*	fut_setup.c */
/*
 * Copyright (C) 2012 Université Bordeaux 1
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

#define CONFIG_FUT 1

#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fut.h>

void *f(void*arg) {
	unsigned i = (uintptr_t) arg;

	if (i)
		while (1)
			FUT_DO_PROBE0(0x1000);
	else
	{
		struct timeval start, end;
		gettimeofday(&start, NULL);
		unsigned n = 0;

		while (1)
		{
			unsigned long diff;
			unsigned j;
			for (j = 0; j < 1000; j++)
				FUT_DO_PROBE0(0x1000);
			n++;
			gettimeofday(&end, NULL);
			diff = end.tv_usec - start.tv_usec
				+ ((end.tv_sec - start.tv_sec) * 1000000);
			if (diff > 1000000)
			{
				printf("%u Kps\n", n);
				n = 0;
				gettimeofday(&start, NULL);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned n, i;
	pthread_t *threads;

	if (argc < 2)
	{
		fprintf(stderr, "I need the number of threads to start\n");
		exit(EXIT_FAILURE);
	}
	n = atoi(argv[1]);
	if (n < 1)
	{
		fprintf(stderr,"I need at least one thread\n");
		exit(EXIT_FAILURE);
	}
	threads = malloc(n * sizeof(*threads));

	fut_set_filename("/dev/null");
	enable_fut_flush();
	fut_setup(64<<20, FUT_KEYMASKALL, pthread_self());
	fut_keychange(FUT_ENABLE, FUT_KEYMASKALL, pthread_self());

	for (i=0; i<n; i++)
		pthread_create(&threads[i], NULL, f, (void*) (uintptr_t) i);
	pthread_exit(NULL);
}
