/*
 * prog.c
 *
 *  Created on: 4 Aug. 2011
 *      Author: Charles Aulagnon <charles.aulagnon@inria.fr>
 *  Modified on: 28 Sep. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "alacon.h"

// Tired of trying to make dyninst work perfectly
#include "libdyninst.c"

int main(int argc, char **argv)
{
	int i, l, s, r;
	if(argc != 3) {
		fprintf(stderr, "usage: %s number_of_iterations matrix_size\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	l = atoi(argv[1]);
	s = atoi(argv[2]);

	for(i = 0; i<l; i++){
		r = foo(s, s);
		assert(r == s+s);
		r = bar();
		assert(r == 42);
	}

	printf("%d\n", l);
	return EXIT_SUCCESS;
}
