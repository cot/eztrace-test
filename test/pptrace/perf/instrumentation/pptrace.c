/*
 * pptrace.c -- full test of pptrace
 *
 *  Created on: 4 Aug. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <stdio.h>
#include <stdlib.h>
#include <pptrace.h>

int main(int argc, char **argv, char **envp)
{
	if(argc < 3) {
		fprintf(stderr, "usage: %s libhijack program [args]\n", argv[0]);
		return EXIT_FAILURE;
	}

	void *bin = pptrace_prepare_binary(argv[2]);
	if(!bin) {
		fprintf(stderr, "Unable to load binary %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	if(pptrace_load_module(bin, argv[1])) {
		fprintf(stderr, "Unable to load module %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	if(pptrace_run(bin, argv+2, envp)) {
		fprintf(stderr, "Unable to run the target\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
