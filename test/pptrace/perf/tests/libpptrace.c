/*
 * libpptrace.c
 *
 *  Created on: 4 Aug. 2011
 *      Author: Charles Aulagnon <charles.aulagnon@inria.fr>
 *      Revised by: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include "hijacks.h"
#include <pptrace_module.h>

int (*orig_foo)(int a, int b);
int (*orig_bar)();

int foo_hijack(int a, int b) {
	hijack_foo_intro(a,b);
	int r = orig_foo(a,b);
	hijack_foo_outro(r,a,b);
	return r;
}

int bar_hijack() {
	hijack_bar_intro();
	int r = orig_bar();
	hijack_bar_outro(r);
	return r;
}

PPTRACE_START_INTERCEPT
	PPTRACE_INTERCEPT_FULL(foo, foo_hijack, orig_foo)
	PPTRACE_INTERCEPT_FULL(bar, bar_hijack, orig_bar)
PPTRACE_END_INTERCEPT
