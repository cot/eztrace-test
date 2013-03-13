/*
 * titi.c -- test case for automated tests
 *
 *  Created on: 2 March. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

char *dummy_symbol[] = { "absolutely", "dummy", "heho", NULL };

int silent = 0;
void postface() {
	if(!silent) printf("In postface()\n");
	if(!silent) printf("Out postface()\n");
}

void preface() {
	if(!silent) printf("In preface()\n");
	if(!silent) printf("Out preface()\n");
}

void tata(char *test)
{
	if(!silent) printf("In tata()\n");
	if(!silent) printf("%s\n", test);
	if(!silent) printf("Out tata()\n");
}

void titi() {
	if(!silent) printf("In titi()\n");
	tata("Hello, World!");
	if(!silent) printf("Out titi()\n");
}

int main(int argc, char **argv) {
	int i;
	int pause = 0;
	for(i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-p") == 0) {
			pause = 1;
		}
		if (strcmp(argv[i], "-s") == 0) {
			silent = 1;
		}
	}
	if(!silent) {
		printf("Entering program with usage : ");
		for(i = 0; i < argc; i++) {
			printf("%s ", argv[i]);
		}
		printf("\n");
	}
	if (pause) {
		printf("Enter a key to continue: ");
		getchar();
		printf("\n");
	}
	titi();
	if(!silent) {
		printf("Exiting program\n");
	}
	return 0;
}

