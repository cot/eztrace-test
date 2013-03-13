/*
 * libpreload.c
 *
 *  Created on: 29 Sep. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <dlfcn.h>
#include "hijacks.h"

int (*orig_foo)(int, int);
int (*orig_bar)();

int foo(int a, int b)
{
	hijack_foo_intro(a, b);
	int r = orig_foo(a, b);
	hijack_foo_outro(a, b, r);
	return r;
}

int bar()
{
	hijack_bar_intro();
	int r = orig_bar();
	hijack_bar_outro(r);
	return r;
}


void __hijack_init (void) __attribute__ ((constructor));
/* Initialize the current library */
void __hijack_init (void)
{
	void* handle = dlopen("libalacon.so", RTLD_NOW);
	if(handle) {
		orig_foo = dlsym(handle, "foo");
		orig_bar = dlsym(handle, "bar");
	}
}
